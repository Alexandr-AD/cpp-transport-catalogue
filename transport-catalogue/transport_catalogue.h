#pragma once

#include <deque>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <set>

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
		std::vector<Stop *> stops;
	};
	void AddBus(Bus &&bus);
	void AddStop(Stop &&stop);
	const Bus *GetBus(std::string_view name) const;
	const Stop *GetStop(std::string_view name) const;
	std::set<std::string_view> StatsOfStop(std::string_view name) const;
	std::vector<std::string> StatsOfBus(const Bus *bus) const;

private:
	std::deque<Bus> buses_;
	std::deque<Stop> stops_;
	std::unordered_map<std::string_view, Bus *> map_buses_;
	std::unordered_map<std::string_view, Stop *> map_stops_;
};

int CountUniqueStops(const TransportCatalogue::Bus &);
int CountStops(const TransportCatalogue::Bus &);
double GetRouteLength(const TransportCatalogue::Bus &);