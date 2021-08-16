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

using Minutes = double;

struct RoutingSettings {
  Minutes bus_wait_time = 0.; // min
  double bus_velocity = 0.; // km/hour
};

struct StopPairVertexId {
  graph::VertexId bus_wait_begin;
  graph::VertexId bus_wait_end;
};

struct WaitEdgeInfo {
  std::string_view stop_name;
  Minutes time = 0.;
};

struct BusEdgeInfo {
  std::string_view bus_name;
  size_t span_count = 0;
  Minutes time = 0.;
};

using EdgeInfo = std::variant<WaitEdgeInfo, BusEdgeInfo>;

struct RouteInfo {
  Minutes total_time = 0.;
  std::vector<EdgeInfo> edges;
};


class TransportRouter{

public:
  TransportRouter() = default;
  TransportRouter(RoutingSettings settings)
      :settings_(std::move(settings)){

  }
//  TransportRouter(graph::DirectedWeightedGraph<Minutes> graf)
//      :graf_(std::move(graf)){

//  }
//  TransportRouter(graph::DirectedWeightedGraph<Minutes> graf, RoutingSettings settings)
//      : settings_(std::move(settings))
//        , graf_(std::move(graf)){

//  }

  void SetRoutingSettings(RoutingSettings settings);

  void SetGraf(graph::DirectedWeightedGraph<Minutes> graf);

  bool BuildRouter(const Catalogue& transport_catalogue);

  std::optional<StopPairVertexId> GetPairVertexId(const transport::Stop* stop) const;

  std::optional<RouteInfo> GetRouteInfo(const graph::VertexId from, graph::VertexId to) const;

  const EdgeInfo& GetEdgeInfo(graph::EdgeId id)const;


private:

  RoutingSettings settings_;
  std::unique_ptr<graph::DirectedWeightedGraph<Minutes>> graf_;
  std::unique_ptr<graph::Router<Minutes>> router_;
  std::unordered_map<const Stop*, StopPairVertexId> stop_ptr_to_pair_id_;
  std::unordered_map<graph::EdgeId, EdgeInfo> edge_id_to_type_;
private:


  void FillGraph(const Catalogue& transport_catalogue);
  void FillStopIdDictionaries(const std::deque<transport::Stop>& stops);
  void AddWaitEdges();
  void AddBusEdges(const Catalogue& transport_catalogue);
  void ParseBusRouteToEdges(const Catalogue& transport_catalogue, const Bus& bus);
  graph::Edge<Minutes> MakeBusEdge(const transport::Stop* from,const transport::Stop* to, const double distance) const;

  template<class Iter>
  void ParseRouteRangeToEdges(Iter first,Iter last,const Catalogue& transport_catalogue,const Bus& bus){
    for (auto iter = first; iter != last; ++iter) {
      const auto from = *iter;

      size_t distance = 0;
      size_t span_count = 0;


      for (auto jter = next(iter); jter <= last; ++jter) {
        if(jter == bus.stops.end())
          break;
        const auto before_to = *prev(jter);
        const auto to = *jter;
        distance += transport_catalogue.GetDistanceBetweenStops(before_to, to);
        ++span_count;

        graph::EdgeId id = graf_->AddEdge(MakeBusEdge(from, to, distance));
        edge_id_to_type_[id] = BusEdgeInfo{bus.name, span_count, graf_->GetEdge(id).weight};
      }

  }
  }

};

}
}
