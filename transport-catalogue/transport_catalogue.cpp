#include <unordered_set>
#include <algorithm>
#include <set>

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
void TransportCatalogue::SetStopDist(std::string_view cur_stop_name, std::string_view dest_stop_name, int dist)
{
    auto cur_stop = GetStop(cur_stop_name);

    auto destination_stop = GetStop(dest_stop_name);

    distance_between_stops_[{cur_stop, destination_stop}] = dist;
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
const Bus *TransportCatalogue::GetBus(std::string_view name) const
{
    auto res = map_buses_.find(name);
    if (res != map_buses_.end())
    {
        return (*res).second;
    }
    return nullptr;
}
const Stop *TransportCatalogue::GetStop(std::string_view name) const
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
                auto it = find_if(bus->stops.begin(), bus->stops.end(), [stop_name](const Stop *lhs)
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

std::optional<BusStats> TransportCatalogue::StatsOfBus(std::string_view bus_name) const
{
    auto bus = GetBus(bus_name);
    if (bus != nullptr)
    {
        BusStats res;
        res.StopsOnRoute = CountStops(*bus);
        res.UniqueStopsOnRoute = CountUniqueStops(*bus);
        res.RouteLength = ComputeRouteLength(*bus);
        res.curvature = res.RouteLength / GetRouteLength(*bus);
        return res;
    }
    return {};
}
int TransportCatalogue::ComputeRouteLength(const Bus &bus) const
{
    int res = 0;
    optional<int> tmp = 0;
    for (size_t i = 0; i < bus.stops.size() - 1; ++i)
    {
        tmp = GetStopDist(bus.stops[i]->name, bus.stops[i + 1]->name);
        if (tmp.has_value())
        {
            res += tmp.value();
        }
    }
    return res;
}

// координаты остановок марщрутов,
// отсортированные по названиям маршрутов
std::vector<std::pair<std::vector<std::pair<geo::Coordinates, std::string>>, std::pair<std::string, bool>>> TransportCatalogue::GetBusStopsCoords(/* const Bus &bus */) const
{
    std::vector<std::pair<std::vector<std::pair<geo::Coordinates, std::string>>, std::pair<std::string, bool>>> res;
    res.reserve(buses_.size());
    std::set<std::string_view> buses_names;
    for (const auto &bus : buses_)
    {
        if (bus.stops.size() > 0)
        {
            buses_names.insert(bus.name);
        }
    }
    for (const auto &bus_name : buses_names)
    {
        std::vector<std::pair<geo::Coordinates, std::string>> stopCoords;
        for (const auto &stop : GetBus(bus_name)->stops)
        {
            stopCoords.push_back({stop->coords, stop->name});
        }
        res.push_back({stopCoords, {bus_name.data(), GetBus(bus_name)->is_roundtrip}});
    }
    return res;
}

std::vector<geo::Coordinates> TransportCatalogue::GetStopsCoords() const
{
    std::vector<geo::Coordinates> res;
    res.reserve(stops_.size());
    std::vector<const Stop*> sort_stops; // = stops_;
    for(auto & stop: stops_) {
        sort_stops.push_back(&stop);
    }
    std::sort(sort_stops.begin(), sort_stops.end(), [](const Stop* lhs, const Stop* rhs) {
        return lhs->name < rhs->name;
    });
    for (const auto &stop : sort_stops)
    {
        for (const auto &bus : buses_)
        {
            if (std::find(bus.stops.begin(), bus.stops.end(), stop) != std::end(bus.stops))
            {
                res.push_back(stop->coords);
                break;
            }
        }
    }
    return res;
}

int CountUniqueStops(const Bus &bus)
{
    std::unordered_set<const Stop *> tmp(bus.stops.begin(), bus.stops.end());
    return tmp.size();
}
int CountStops(const Bus &bus)
{
    return bus.stops.size();
}
double GetRouteLength(const Bus &bus)
{
    double res = 0;
    for (size_t i = 0; i < bus.stops.size() - 1; ++i)
    {
        res += geo::ComputeDistance(bus.stops[i]->coords, bus.stops[i + 1]->coords);
    }
    return res;
}