#include <unordered_set>
#include <algorithm>

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
TransportCatalogue::Bus *TransportCatalogue::GetBus(std::string_view name) const
{
    if (map_buses_.count(name.data()))
    {
        return map_buses_.at(name.data());
    }
    return nullptr;
}
TransportCatalogue::Stop *TransportCatalogue::GetStop(std::string_view name) const
{
    if (map_stops_.count(name))
    {
        return map_stops_.at(name);
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
                auto it = find_if(bus->stops.begin(), bus->stops.end(), [stop_name](TransportCatalogue::Stop *lhs)
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

int CountUniqueStops(const TransportCatalogue::Bus &bus)
{
    std::unordered_set<TransportCatalogue::Stop *> tmp(bus.stops.begin(), bus.stops.end());
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