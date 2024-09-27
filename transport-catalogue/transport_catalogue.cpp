#include <unordered_set>
#include <algorithm>
// #include <string>

#include "transport_catalogue.h"

using namespace std;

void TransportCatalogue::AddBus(Bus &&bus)
{
    buses_.push_back(std::move(bus));
    map_buses_[buses_.back().name] = &buses_.back();
}
void TransportCatalogue::AddStop(Stop &&stop)
{
    stops_.push_back(std::move(stop));
    map_stops_[stops_.back().name] = &stops_.back();
}
void TransportCatalogue::AddStopDist(std::string_view cur_stop_name, std::string_view command_descr)
{
    string tmp_string = command_descr.substr(command_descr.find(',', command_descr.find(',') + 1) + 2).data();
    auto cur_stop = GetStop(cur_stop_name);

    while (!tmp_string.empty())
    {
        string tmp;
        size_t num_size = tmp_string.find('m');

        int dist_btn_stops = stoi(tmp_string.substr(0, num_size));

        tmp_string = tmp_string.substr(num_size + 5);

        auto pos_comma = tmp_string.find(',');
        if (pos_comma < 100)
            tmp = tmp_string.substr(0, pos_comma);
        auto destination_stop = pos_comma != tmp_string.npos ? GetStop(tmp_string.substr(0, pos_comma)) : GetStop(tmp_string);

        distance_between_stops_[{cur_stop, destination_stop}] = dist_btn_stops;

        if (tmp_string.find(',') == tmp_string.npos)
        {
            tmp_string.clear();
        }
        else
        {
            tmp_string = tmp_string.substr(tmp_string.find(',') + 2);
        }
    }
}
std::optional<int> TransportCatalogue::GetStopDist(std::string_view begin_stop_name, std::string_view dest_stop_name) const
{
    auto begin_stop = GetStop(begin_stop_name),
         dest_stop = GetStop(dest_stop_name);
    if (distance_between_stops_.count({begin_stop, dest_stop}))
    {
        return distance_between_stops_.at({begin_stop, dest_stop});
    }
    else if (distance_between_stops_.count({dest_stop, begin_stop}))
    {
        return distance_between_stops_.at({dest_stop, begin_stop});
    }
    return nullopt;
}
const TransportCatalogue::Bus *TransportCatalogue::GetBus(std::string_view name) const
{
    auto res = map_buses_.find(name);
    if (res != map_buses_.end())
    {
        return (*res).second;
    }
    return nullptr;
}
const TransportCatalogue::Stop *TransportCatalogue::GetStop(std::string_view name) const
{
    auto res = map_stops_.find(name);
    if (res != map_stops_.end())
    {
        return (*res).second;
    }
    return nullptr;
}
std::set<std::string_view> TransportCatalogue::StatsOfStop(std::string_view stop_name) const
{
    if (map_stops_.at(stop_name)->buses.size() == 0)
    {
        std::set<std::string_view> names_buses;
        for (const auto &[bus_name, bus] : map_buses_)
        {
            if (bus->stops.size())
            {
                auto it = find_if(bus->stops.begin(), bus->stops.end(), [stop_name](const TransportCatalogue::Stop *lhs)
                                  { return lhs->name == stop_name; });
                if (it != bus->stops.end())
                {
                    names_buses.insert(bus->name);
                }
            }
        }
        map_stops_.at(stop_name)->buses = move(names_buses);
    }
    return map_stops_.at(stop_name)->buses;
}

std::optional<TransportCatalogue::BusStats> TransportCatalogue::StatsOfBus(std::string_view bus_name) const
{
    auto bus = GetBus(bus_name);
    if (bus != nullptr)
    {
        BusStats res;
        res.StopsOnRoute = CountStops(*bus);
        res.UniqueStopsOnRoute = CountUniqueStops(*bus);
        res.RouteLength = ComputeRouteLength(*bus);
        res.curvature = res.RouteLength/GetRouteLength(*bus);
        return res;
    }
    return {};
}
int TransportCatalogue::ComputeRouteLength(const Bus & bus) const
{
    int res = 0;
    optional<int> tmp = 0;
    for (size_t i = 0; i < bus.stops.size() - 1; ++i)
    {
        tmp = GetStopDist(bus.stops[i]->name, bus.stops[i+1]->name);
        if(tmp.has_value()) {
            res += tmp.value();
        }
    }
    return res;
}

int CountUniqueStops(const TransportCatalogue::Bus &bus)
{
    std::unordered_set<const TransportCatalogue::Stop *> tmp(bus.stops.begin(), bus.stops.end());
    return tmp.size();
}
int CountStops(const TransportCatalogue::Bus &bus)
{
    return bus.stops.size();
}
double GetRouteLength(const TransportCatalogue::Bus &bus)
{
    double res = 0;
    for (size_t i = 0; i < bus.stops.size() - 1; ++i)
    {
        res += geo::ComputeDistance(bus.stops[i]->coords, bus.stops[i + 1]->coords);
    }
    return res;
}