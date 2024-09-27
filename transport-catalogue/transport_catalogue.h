#pragma once

#include <deque>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <set>
#include <optional>

#include "geo.h"

class TransportCatalogue
{
public:
	struct Stop
	{
		std::string name;
		geo::Coordinates coords;
		std::set<std::string_view> buses;
	};
	struct Bus
	{
		std::string name;
		std::vector<const Stop *> stops;
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
		size_t operator()(const std::pair<const Stop *, const Stop *> &p) const
		{
			return std::hash<const void *>()(p.first) ^ std::hash<const void *>()(p.second);
		}
	};

	void AddBus(Bus &&bus);
	void AddStop(Stop &&stop);
	void AddStopDist(std::string_view stop_name, std::string_view command_descr);
	std::optional<int> GetStopDist(std::string_view begin_stop_name, std::string_view dest_stop_name) const;
	const Bus *GetBus(std::string_view name) const;
	const Stop *GetStop(std::string_view name) const;
	int ComputeRouteLength(const Bus &) const;
	std::set<std::string_view> StatsOfStop(std::string_view name) const;
	std::optional<TransportCatalogue::BusStats> StatsOfBus(std::string_view bus_name) const;

private:
	std::deque<Bus> buses_;
	std::deque<Stop> stops_;
	std::unordered_map<std::string_view, Bus *> map_buses_;
	std::unordered_map<std::string_view, Stop *> map_stops_;
	std::unordered_map<std::pair<const Stop *, const Stop *>, int, StopsDistanceHasher> distance_between_stops_;
};

int CountUniqueStops(const TransportCatalogue::Bus &);
int CountStops(const TransportCatalogue::Bus &);
double GetRouteLength(const TransportCatalogue::Bus &);
