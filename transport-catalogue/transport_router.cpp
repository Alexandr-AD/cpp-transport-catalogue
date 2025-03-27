#include "geo.h"
#include "transport_router.h"
// #include "json_reader.h"
#include "json_builder.h"

#include <exception>

// #include <iostream> //удалить

using namespace std;

const graph::DirectedWeightedGraph<double> &routing::TransportRouter::BuildGraph(const TransportCatalogue &catalogue)
{
    // if (!graph_.has_value())
    // {
    graph::DirectedWeightedGraph<double> res_graph(2 * catalogue.GetStopsCount());

    int stop_from_vertex = 0;
    int stop_to_vertex = 1;

    // создание словаря инлексов вершин остановок и индексов
    for (const auto &stop : catalogue.GetAllStops())
    {
        stop_as_pair_number_[stop.name] = {stop_from_vertex, stop_to_vertex};
        number_as_stop_name[stop_from_vertex] = stop.name;
        number_as_stop_name[stop_to_vertex] = stop.name;

        stop_from_vertex += 2;
        stop_to_vertex += 2;
    }
    // связи вершин ожидания и отправления одной остановки
    for (const auto &[name, vertexes] : stop_as_pair_number_)
    {
        graph::Edge<double> edge{
            static_cast<graph::VertexId>(vertexes.first),
            static_cast<graph::VertexId>(vertexes.second),
            static_cast<double>(settings_.bus_wait_time_),
            0,
            ""};

        int edgeId = res_graph.AddEdge(edge);
        edge_id_to_type_[edgeId] = EdgeType::WAIT;
    }
    // построение всех путей
    for (const auto &bus : catalogue.GetBuses())
    {
        for (int i = 0; i < bus.stops.size(); ++i)
        {
            const Stop *stop_from = bus.stops[i];
            int distance = 0;
            int reverse_distanse = 0;
            for (int j = i + 1; j < bus.stops.size(); ++j)
            {
                const Stop *stop_from_tmp = bus.stops[j - 1];
                const Stop *stop_to = bus.stops[j];

                std::pair<int, int> stop_from_nums = stop_as_pair_number_[stop_from->name];
                std::pair<int, int> stop_to_nums = stop_as_pair_number_[stop_to->name];

                auto dist = catalogue.GetStopDist(stop_from_tmp->name, stop_to->name);
                distance += *dist;

                graph::Edge<double> edge;
                edge.from = stop_from_nums.second;
                edge.to = stop_to_nums.first;
                edge.weight = distance / (settings_.bus_velocity_ * (100.0 / 6.0));
                edge.span_count = j - i;
                edge.bus = bus.name;

                int edgeId = res_graph.AddEdge(edge);
                edge_id_to_type_[edgeId] = EdgeType::ROUTE;

                if (!bus.is_roundtrip)
                {
                    dist = catalogue.GetStopDist(stop_to->name, stop_from_tmp->name);
                    reverse_distanse += *dist;

                    graph::Edge<double> edge_reverse;
                    edge_reverse.from = stop_to_nums.second;
                    edge_reverse.to = stop_from_nums.first;
                    edge_reverse.weight = reverse_distanse / (settings_.bus_velocity_ * (100.0 / 6.0));
                    edge_reverse.span_count = j - i;
                    edge_reverse.bus = bus.name;

                    int edgeId_inner = res_graph.AddEdge(edge_reverse);
                    edge_id_to_type_[edgeId_inner] = EdgeType::ROUTE;
                }
            }
        }
    }

    graph_ = std::move(res_graph);
    // }
    return graph_;
}

const std::unordered_map<std::string_view, std::pair<int, int>> &routing::TransportRouter::GetStopsAsNumber() const
{
    return stop_as_pair_number_;
}
const std::unordered_map<int, std::string_view> &routing::TransportRouter::GetNumsAsStopName() const
{
    return number_as_stop_name;
}
const std::unordered_map<int, routing::EdgeType> &routing::TransportRouter::GetEdgeType() const
{
    return edge_id_to_type_;
}
const routing::RoutingSettings &routing::TransportRouter::GetSettings() const
{
    return settings_;
}
void routing::TransportRouter::SetSettings(routing::RoutingSettings &&settings)
{
    settings_ = settings;
}

const graph::Edge<double> routing::TransportRouter::GetEdgeInfo(int edgeId) const
{
    return graph_.GetEdge(edgeId);
}
