#include <algorithm>
#include <cassert>
#include <iterator>
#include <map>
#include <sstream>

#include "json_reader.h"
#include "map_renderer.h"
#include "json_builder.h"

void catalogueInput::CommandDescription::AddBusesDescr(std::vector<BusDescr> busDescr)
{
    busDescr_ = std::move(busDescr);
}
void catalogueInput::CommandDescription::AddStopsDescr(std::vector<StopDescr> stopsDescr)
{
    stopsDescr_ = std::move(stopsDescr);
}

void catalogueInput::CommandDescription::AddStopDescr(catalogueInput::StopDescr stopsDescr)
{
    stopsDescr_.push_back(std::move(stopsDescr));
}
void catalogueInput::CommandDescription::AddBusDescr(catalogueInput::BusDescr busDescr)
{
    busDescr_.push_back(std::move(busDescr));
}
void catalogueInput::CommandDescription::AddBaseRequest(StatRequest request)
{
    requests_.push_back(std::move(request));
}
std::vector<catalogueInput::StopDescr> catalogueInput::CommandDescription::GetStopsDescr() const
{
    return stopsDescr_;
}
std::vector<catalogueInput::BusDescr> catalogueInput::CommandDescription::GetBusesDescr() const
{
    return busDescr_;
}

const svg::Color ReadColor(const json::Node &node)
{
    svg::Color color;
    if (node.IsArray())
    {
        if (node.AsArray().size() == 3)
        {
            svg::RGB tmp = {
                node.AsArray()[0].AsInt(),
                node.AsArray()[1].AsInt(),
                node.AsArray()[2].AsInt()};
            color = std::move(tmp);
        }
        else
        {
            svg::RGBA tmp = {
                node.AsArray()[0].AsInt(),
                node.AsArray()[1].AsInt(),
                node.AsArray()[2].AsInt(),
                node.AsArray()[3].AsDouble()};
            color = std::move(tmp);
        }
    }
    else if (node.IsString())
    {
        color = node.AsString();
    }

    return color;
}

renderer::RenderSettings catalogueInput::ReadRenderSettings(const json::Document &doc)
{
    auto inpMap = doc.GetRoot().AsMap().at("render_settings").AsMap();

    renderer::RenderSettings settings;

    settings.width_ = inpMap.at("width").AsDouble();
    settings.height_ = inpMap.at("height").AsDouble();
    settings.padding_ = inpMap.at("padding").AsDouble();
    settings.line_width_ = inpMap.at("line_width").AsDouble();
    settings.stop_radius_ = inpMap.at("stop_radius").AsDouble();
    settings.bus_label_font_size_ = inpMap.at("bus_label_font_size").AsInt();
    settings.bus_label_offset_ = {inpMap.at("bus_label_offset").AsArray()[0].AsDouble(), inpMap.at("bus_label_offset").AsArray()[1].AsDouble()};
    settings.stop_label_font_size_ = inpMap.at("stop_label_font_size").AsInt();
    settings.stop_label_offset_ = {inpMap.at("stop_label_offset").AsArray()[0].AsDouble(), inpMap.at("stop_label_offset").AsArray()[1].AsDouble()};
    settings.underlayer_width_ = inpMap.at("underlayer_width").AsDouble();

    settings.underlayer_color_ = ReadColor(inpMap.at("underlayer_color"));
    for (const auto &color : inpMap.at("color_palette").AsArray())
    {
        settings.color_palette_.push_back(ReadColor(color));
    }

    return settings;
}

void catalogueInput::InputReader::ApplyCommands(TransportCatalogue &catalogue, const catalogueInput::CommandDescription &cmds) const
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

catalogueInput::CommandDescription catalogueInput::InputReader::ParseCommandInput(json::Document doc)
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
    routing::TransportRouter transportRouter;

    auto settings = catalogueInput::ReadRoutingSettings(inpDoc);
    transportRouter.SetSettings(std::move(settings));
    transportRouter.BuildGraph(transport_catalogue);

    for (const auto &request : requests)
    {
        if (request.AsMap().at("type").AsString() == "Stop")
        {
            auto stop = transport_catalogue.GetStop(request.AsMap().at("name").AsString());
            if (stop == nullptr)
            {
                json::Node errNode(
                    json::Builder{}
                        .StartDict()
                        .Key("request_id"s)
                        .Value(request.AsMap().at("id"s).AsInt())
                        .Key("error_message"s)
                        .Value("not found"s)
                        .EndDict()
                        .Build());
                res.push_back(std::move(errNode));
            }
            else
            {
                json::Array buses;
                for (const auto &bus_name : transport_catalogue.StatsOfStop(request.AsMap().at("name"s).AsString()))
                {
                    buses.push_back({std::string(bus_name)});
                }
                json::Node stopStats(
                    json::Builder{}
                        .StartDict()
                        .Key("buses"s)
                        .Value(buses)
                        .Key("request_id"s)
                        .Value(request.AsMap().at("id"s).AsInt())
                        .EndDict()
                        .Build());
                res.push_back(std::move(stopStats));
            }
        }
        if (request.AsMap().at("type"s).AsString() == "Bus"s)
        {
            auto bus = transport_catalogue.GetBus(request.AsMap().at("name").AsString());
            if (bus == nullptr)
            {
                json::Node errNode(
                    json::Builder{}
                        .StartDict()
                        .Key("request_id"s)
                        .Value(request.AsMap().at("id"s).AsInt())
                        .Key("error_message"s)
                        .Value("not found"s)
                        .EndDict()
                        .Build());
                res.push_back(std::move(errNode));
            }
            else
            {
                auto bus_stats = transport_catalogue.StatsOfBus(request.AsMap().at("name"s).AsString());
                res.push_back(
                    json::Builder{}
                        .StartDict()
                        .Key("curvature"s)
                        .Value(bus_stats.value().curvature)
                        .Key("request_id"s)
                        .Value(request.AsMap().at("id"s).AsInt())
                        .Key("route_length"s)
                        .Value(bus_stats.value().RouteLength)
                        .Key("stop_count"s)
                        .Value(bus_stats.value().StopsOnRoute)
                        .Key("unique_stop_count"s)
                        .Value(bus_stats.value().UniqueStopsOnRoute)
                        .EndDict()
                        .Build());
            }
        }
        if (request.AsMap().at("type"s).AsString() == "Map"s)
        {
            std::ostringstream outStr;
            auto settings = catalogueInput::ReadRenderSettings(inpDoc);
            auto allCoords = transport_catalogue.GetStopsCoords();
            auto busCoords = transport_catalogue.GetBusStopsCoords();
            renderer::MapRenderer(settings, allCoords, busCoords, outStr);
            res.push_back(
                json::Builder{}
                    .StartDict()
                    .Key("map"s)
                    .Value(outStr.str())
                    .Key("request_id"s)
                    .Value(request.AsMap().at("id"s).AsInt())
                    .EndDict()
                    .Build());
        }
        if (request.AsMap().at("type"s).AsString() == "Route"s)
        {

            std::string stopFrom = request.AsMap().at("from"s).AsString();
            std::string stopTo = request.AsMap().at("to"s).AsString();

            auto routeItems = transportRouter.FindRoute(stopFrom, stopTo);

            if (!routeItems.has_value())
            {
                json::Node errNode(
                    json::Builder{}
                        .StartDict()
                        .Key("request_id"s)
                        .Value(request.AsMap().at("id"s).AsInt())
                        .Key("error_message"s)
                        .Value("not found"s)
                        .EndDict()
                        .Build());
                res.push_back(errNode);
            }
            else
            {
                json::Array items;
                double routeTime = 0;
                for (const auto &item : routeItems.value())
                {
                    // if (item.type_ == routing::EdgeType::WAIT)
                    if (std::holds_alternative<routing::WaitItem>(item))
                    {
                        auto &tmp = std::get<routing::WaitItem>(item);
                        routeTime += tmp.time_;

                        items.push_back(
                            json::Builder{}
                                .StartDict()
                                .Key("type"s)
                                .Value("Wait"s)
                                .Key("stop_name"s)
                                .Value(tmp.stop_name_)
                                .Key("time"s)
                                .Value(tmp.time_)
                                .EndDict()
                                .Build());
                    }
                    else
                    {
                        auto &tmp = std::get<routing::BusItem>(item);
                        routeTime += tmp.time_;

                        items.push_back(
                            json::Builder{}
                                .StartDict()
                                .Key("type"s)
                                .Value("Bus"s)
                                .Key("bus"s)
                                .Value(tmp.bus_)
                                .Key("span_count"s)
                                .Value(tmp.span_count_)
                                .Key("time"s)
                                .Value(tmp.time_)
                                .EndDict()
                                .Build());
                    }
                }
                res.push_back(
                    json::Builder{}
                        .StartDict()
                        .Key("request_id"s)
                        .Value(request.AsMap().at("id"s).AsInt())
                        .Key("total_time"s)
                        .Value(routeTime)
                        .Key("items"s)
                        .Value(items)
                        .EndDict()
                        .Build());
            }

        }
    }
    json::Document doc(res);

    return doc;
}

routing::RoutingSettings catalogueInput::ReadRoutingSettings(const json::Document &doc)
{
    auto inpMap = doc.GetRoot().AsMap().at("routing_settings").AsMap();
    int bus_wait_time = inpMap.at("bus_wait_time").AsInt();
    int bus_velocity = inpMap.at("bus_velocity").AsInt();

    return routing::RoutingSettings{bus_wait_time, bus_velocity};
}
// /*
//  * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
//  * а также код обработки запросов к базе и формирование массива ответов в формате JSON
//  */
