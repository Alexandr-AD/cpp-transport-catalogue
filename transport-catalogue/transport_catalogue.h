#pragma once

#include <deque>
#include <unordered_map>
#include <optional>

#include "domain.h"

class TransportCatalogue
{
public:
	void AddBus(Bus &&bus);
	void AddStop(Stop &&stop);
	void SetStopDist(std::string_view cur_stop_name, std::string_view destination_stop, int dist);
	std::optional<int> GetStopDist(std::string_view begin_stop_name, std::string_view dest_stop_name) const;
	int ComputeDistBtwnStops(int begin_stop, int dest_stop, const Bus &bus) const;
	const Bus *GetBus(std::string_view name) const;
	const Stop *GetStop(std::string_view name) const;
	std::vector<std::pair<std::vector<std::pair<geo::Coordinates, std::string>>, std::pair<std::string, bool>>> GetBusStopsCoords() const;
	std::vector<geo::Coordinates> GetStopsCoords() const;
	int ComputeRouteLength(const Bus &) const;
	std::set<std::string_view> StatsOfStop(std::string_view name) const;
	std::optional<BusStats> StatsOfBus(std::string_view bus_name) const;
	int GetStopsCount() const;
	int GetBusesCount() const;
	const std::deque<Bus> &GetBuses() const;
	const std::deque<Stop> &GetAllStops() const;

private:
	std::deque<Bus> buses_;
	std::deque<Stop> stops_;
	std::unordered_map<std::string_view, Bus *> map_buses_;
	std::unordered_map<std::string_view, Stop *> map_stops_;
	std::unordered_map<std::pair<const Stop *, const Stop *>, int, StopsDistanceHasher> distance_between_stops_;
};

int CountUniqueStops(const Bus &);
int CountStops(const Bus &);
double GetRouteLength(const Bus &);
