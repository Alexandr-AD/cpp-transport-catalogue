#pragma once

#include <utility>
#include <set>
#include <string_view>
#include <string>
#include <vector>

#include "geo.h"

struct Stop
{
    std::string name;
    geo::Coordinates coords;
    std::set<std::string_view> buses;

    bool operator==(const Stop &other)
    {
        return name == other.name &&
               coords == other.coords;
    }
};
struct Bus
{
    std::string name;
    std::vector<const Stop *> stops;
    bool is_roundtrip;
};
struct BusStats
{
    int StopsOnRoute;
    int UniqueStopsOnRoute;
    double RouteLength;
    double curvature;
};
struct StopsDistanceHasher
{
    size_t operator()(const std::pair<const Stop *, const Stop *> &p) const;
};