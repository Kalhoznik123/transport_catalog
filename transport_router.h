#pragma once
#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"
#include "domain.h"
#include <optional>
#include <vector>
#include <memory>
#include <variant>
#include <unordered_map>
namespace transport {

namespace router {


struct RoutingSettings {
    int bus_wait_time = 0.;
    double bus_velocity = 0.;
};



struct WaitEdgeInfo {
    std::string stop_name;
    //std::string_view stop_name;
    double time = 0.;
};

struct BusEdgeInfo {
    std::string bus_name;
    //std::string_view bus_name;
    size_t span_count = 0;
    double time = 0.;
};

using EdgeInfo = std::variant<WaitEdgeInfo, BusEdgeInfo>;

struct RouteInfo {
    double total_time = 0.;
    std::vector<EdgeInfo> edges;
};

struct StopPairVertexId {
    graph::VertexId bus_wait_begin;
    graph::VertexId bus_wait_end;
};

class TransportRouter{

public:
    TransportRouter() = default;
    TransportRouter(RoutingSettings settings)
        :settings_(std::move(settings)){

    }

    void SetRoutingSettings(RoutingSettings settings);

    void BuildRouter(const Catalogue& transport_catalogue);

    void BuildRouter(std::vector<graph::Edge<double>> edges,
                     std::vector<std::vector<graph::EdgeId>> incidence_lists,
                     std::unordered_map<const Stop*, StopPairVertexId> stop_ptr_to_pair_id,
                     std::unordered_map<graph::EdgeId, EdgeInfo> edge_id_to_type);

    std::optional<RouteInfo> GetRouteInfo(const transport::Stop* from, const transport::Stop* to) const;

    const std::vector<graph::Edge<double>>& GetGrafEdges() const ;
    const std::vector<std::vector<graph::EdgeId>>& GetGrafIncidenceLists() const;
    const std::unordered_map<const Stop*, StopPairVertexId>&  GetStopToPairID() const;
    const std::unordered_map<graph::EdgeId, EdgeInfo>& GetEdgeIdToType() const;
    const RoutingSettings& GetRoutingSettings() const;
private:

    RoutingSettings settings_;
    std::unique_ptr<graph::DirectedWeightedGraph<double>> router_graf_;
    std::unique_ptr<graph::Router<double>> router_;
    std::unordered_map<const Stop*, StopPairVertexId> stop_ptr_to_pair_id_;
    std::unordered_map<graph::EdgeId, EdgeInfo> edge_id_to_type_;
private:


    std::optional<StopPairVertexId> GetPairVertexId(const transport::Stop* stop) const;
    const EdgeInfo& GetEdgeInfo(graph::EdgeId id)const;
    void FillGraph(const Catalogue& transport_catalogue);
    void FillStopIdDictionaries(const std::deque<transport::Stop>& stops);
    void AddWaitEdges();
    void AddBusEdges(const Catalogue& transport_catalogue);
    void ParseBusRouteToEdges(const Catalogue& transport_catalogue, const Bus& bus);
    void AddToStopPtrPairId(const Stop* key, const StopPairVertexId& value);
    void AddToEdgeIdToType(graph::EdgeId key, const EdgeInfo& value);
    void SetStopPtdToPairId(std::unordered_map<const Stop*, StopPairVertexId> stop_ptr_to_pair_id);
    void SetEdgeIdToType(std::unordered_map<graph::EdgeId, EdgeInfo> edge_id_to_type);
    graph::Edge<double> MakeBusEdge(const transport::Stop* from,const transport::Stop* to, const double distance) const;

    template<class RandomIter>
    void FillTransportRouter(RandomIter first,RandomIter last,const Catalogue& transport_catalogue,const Bus& bus){
        for (auto iter = first; iter != last; ++iter) {
            const auto from = *iter;

            size_t distance = 0;
            size_t span_count = 0;


            for (auto jter = next(iter); jter != last; ++jter) {

                const auto before_to = *prev(jter);
                const auto to = *jter;
                distance += transport_catalogue.GetDistanceBetweenStops(before_to, to);
                ++span_count;

                const  graph::EdgeId id = router_graf_->AddEdge(MakeBusEdge(from, to, distance));
                edge_id_to_type_[id] = BusEdgeInfo{bus.name, span_count, router_graf_->GetEdge(id).weight};
            }

        }
    }

};

}
}
