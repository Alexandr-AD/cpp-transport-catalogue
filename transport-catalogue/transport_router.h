#pragma once

#include <string>

#include "json.h"
#include "graph.h"
#include "transport_catalogue.h"
#include "router.h"

namespace routing
{

    enum class EdgeType
    {
        WAIT,
        ROUTE
    };
    struct RoutingSettings
    {
        int bus_wait_time_ = 1;
        int bus_velocity_ = 1;
    };

    struct WaitItem
    {
        std::string stop_name_;
        double time_ = 0;
    };
    struct BusItem
    {
        std::string bus_;
        int span_count_ = 0;
        double time_ = 0;
    };
    using RouteItems = std::variant<WaitItem, BusItem>;
    class TransportRouter
    {
    public:
        TransportRouter() = default;
        void BuildGraph(const TransportCatalogue &catalogue);

        const std::optional<std::vector<RouteItems>> FindRoute(std::string_view from, std::string_view to) const;

        void SetSettings(RoutingSettings &&settings);

    private:
        const std::unordered_map<int, EdgeType> &GetEdgeType() const;
        const std::unordered_map<std::string_view, std::pair<int, int>> &GetStopsAsNumber() const;
        const std::unordered_map<int, std::string_view> &GetNumsAsStopName() const;
        const graph::Edge<double> GetEdgeInfo(int edgeId) const;
        const RoutingSettings &GetSettings() const;

        RoutingSettings settings_;
        graph::DirectedWeightedGraph<double> graph_;
        std::unordered_map<std::string_view, std::pair<int, int>> stop_as_pair_number_;
        std::unordered_map<int, std::string_view> number_as_stop_name;
        std::unordered_map<int, EdgeType> edge_id_to_type_;

        graph::Router<double> *router_;
    };
} // namespace routing