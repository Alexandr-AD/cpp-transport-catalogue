#include <sstream>

#include "map_renderer.h"
#include "request_handler.h"
#include "json_reader.h"

void InputReader::ApplyCommands([[maybe_unused]] TransportCatalogue &catalogue, catalogueInput::CommandDescription cmds) const
{
    using namespace std::literals;
    for (const auto &command : cmds.GetStopsDescr())
    {
        catalogue.AddStop({command.name_, command.coords_, {}});
    }
    for (const auto &command : cmds.GetStopsDescr())
    {
        for (const auto &[dest_stop, dist] : command.distance_to_stops_)
        {
            catalogue.SetStopDist(command.name_, dest_stop, dist);
        }
    }
    for (const auto &command : cmds.GetBusesDescr())
    {
        std::vector<const Stop *> stops_ptr;

        for (std::string_view s : command.stops_)
        {
            const auto stop = catalogue.GetStop(s);
            if (stop != nullptr)
            {
                stops_ptr.push_back(stop);
            }
        }

        if (!command.is_roundtrip_)
        {
            if (command.stops_.size() > 1)
            {
                for (int i = command.stops_.size() - 2; i >= 0; --i)
                {
                    const auto stop = catalogue.GetStop(command.stops_[i]);
                    if (stop != nullptr)
                    {
                        stops_ptr.push_back(stop);
                    }
                }
            }
        }
        catalogue.AddBus({command.name_, stops_ptr, command.is_roundtrip_});
    }
}

catalogueInput::CommandDescription InputReader::ParseCommandInput(json::Document doc)
{
    catalogueInput::CommandDescription res;

    for (const auto &node : doc.GetRoot().AsMap().at("base_requests").AsArray())
    {
        auto tmp = node.AsMap();
        if (tmp.at("type").AsString() == "Stop")
        {
            std::string name = tmp.at("name").AsString();
            double latitude = tmp.at("latitude").AsDouble();
            double longitude = tmp.at("longitude").AsDouble();
            std::map<std::string, int> distance_to_stops;
            for (const auto &[stop, dist] : tmp.at("road_distances").AsMap())
            {
                distance_to_stops[stop] = dist.AsInt();
            }

            res.AddStopDescr({name,
                              {
                                  latitude,
                                  longitude,
                              },
                              distance_to_stops});
        }
        else if (tmp.at("type").AsString() == "Bus")
        {
            std::string name = tmp.at("name").AsString();
            std::vector<std::string> stops;

            for (const auto &stop : tmp.at("stops").AsArray())
            {
                stops.push_back(stop.AsString());
            }
            bool is_roundtrip = tmp.at("is_roundtrip").AsBool();
            res.AddBusDescr({name,
                             stops,
                             is_roundtrip});
        }
    }

    return res;
}
json::Document printStat::PrintStats(const TransportCatalogue &transport_catalogue, json::Array requests, const json::Document &inpDoc)
{
    using namespace std::literals;
    json::Array res;
    for (const auto &request : requests)
    {
        if (request.AsMap().at("type").AsString() == "Stop")
        {
            auto stop = transport_catalogue.GetStop(request.AsMap().at("name").AsString());
            if (stop == nullptr)
            {
                json::Node errNode({{"request_id"s, json::Node(request.AsMap().at("id"s).AsInt())},
                                    {"error_message"s, json::Node("not found"s)}});
                res.push_back(std::move(errNode));
            }
            else
            {
                json::Array buses;
                for (const auto &bus_name : transport_catalogue.StatsOfStop(request.AsMap().at("name"s).AsString()))
                {
                    buses.push_back({std::string(bus_name)});
                }
                json::Node stopStats({{"buses"s, json::Node(buses)},
                                      {"request_id"s, json::Node(request.AsMap().at("id"s).AsInt())}});
                res.push_back(std::move(stopStats));
            }
        }
        if (request.AsMap().at("type"s).AsString() == "Bus"s)
        {
            auto bus = transport_catalogue.GetBus(request.AsMap().at("name").AsString());
            if (bus == nullptr)
            {
                json::Node errNode({{"request_id"s, json::Node(request.AsMap().at("id"s).AsInt())},
                                    {"error_message"s, json::Node("not found"s)}});
                res.push_back(std::move(errNode));
            }
            else
            {
                auto bus_stats = transport_catalogue.StatsOfBus(request.AsMap().at("name"s).AsString());
                res.push_back(json::Node({{"curvature"s, bus_stats.value().curvature},
                                          {"request_id"s, request.AsMap().at("id"s).AsInt()},
                                          {"route_length"s, bus_stats.value().RouteLength},
                                          {"stop_count"s, bus_stats.value().StopsOnRoute},
                                          {"unique_stop_count"s, bus_stats.value().UniqueStopsOnRoute}}));
            }
        }
        if (request.AsMap().at("type"s).AsString() == "Map"s)
        {
            std::ostringstream outStr;
            auto settings = catalogueInput::read_settings(inpDoc);
            auto allCoords = transport_catalogue.GetStopsCoords();
            auto busCoords = transport_catalogue.GetBusStopsCoords();
            renderer::map_renderer(settings, allCoords, busCoords, outStr);
            res.push_back(json::Node({{"map"s, outStr.str()},
                                      {"request_id"s, request.AsMap().at("id"s).AsInt()}}));
        }
    }
    json::Document doc(res);

    return doc;
}

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */