#include "transport_router.h"

namespace transport {

namespace router {

void TransportRouter::SetRoutingSettings(RoutingSettings settings){
  settings_ = std::move(settings);
}


void TransportRouter::BuildRouter(const Catalogue &transport_catalogue){

  router_graf_ = std::make_unique<graph::DirectedWeightedGraph<double>>(transport_catalogue.GetStops().size() * 2);
  FillGraph(transport_catalogue);
  router_ = std::make_unique<graph::Router<double>>(*router_graf_);

}

std::optional<TransportRouter::StopPairVertexId> TransportRouter::GetPairVertexId(const Stop *stop) const{
  const auto it  = stop_ptr_to_pair_id_.find(stop);

  if(it == stop_ptr_to_pair_id_.end())
    return std::nullopt;

  return it->second;

  //return  stop_ptr_to_pair_id_.at(stop);
}

const EdgeInfo &TransportRouter::GetEdgeInfo(graph::EdgeId id) const{
  return  edge_id_to_type_.at(id);
}

std::optional<RouteInfo> TransportRouter::GetRouteInfo(const Stop* from, const Stop* to) const{
  if(router_ == nullptr)
     return  std::nullopt;

  const auto from_id = GetPairVertexId(from);
  const auto to_id = GetPairVertexId(to);

  if(!from_id || !to_id)
    return std::nullopt;

  auto route_info = router_->BuildRoute(from_id->bus_wait_begin,to_id->bus_wait_begin);

  if(!route_info)
    return std::nullopt;

  RouteInfo result;
  result.total_time = route_info->weight;
  for(graph::EdgeId& edge :route_info->edges){
    result.edges.push_back(GetEdgeInfo(std::move(edge)));
  }

  return  result;

}

void TransportRouter::FillGraph(const Catalogue &transport_catalogue) {
  FillStopIdDictionaries(transport_catalogue.GetStops());
  AddWaitEdges();
  AddBusEdges(transport_catalogue);
}

void TransportRouter::FillStopIdDictionaries(const std::deque<Stop> &stops) {
  size_t i = 0;
  for (const auto& stop : stops) {
    const graph::VertexId first_id = i++;
    const graph::VertexId second_id = i++;
    stop_ptr_to_pair_id_[&stop] = StopPairVertexId{first_id, second_id};
  }
}

void TransportRouter::AddWaitEdges() {
  for (const auto& [stop_ptr, ids] : stop_ptr_to_pair_id_) {
    const  graph::EdgeId id = router_graf_->AddEdge(graph::Edge<double>{
        ids.bus_wait_begin, ids.bus_wait_end, static_cast<double>(settings_.bus_wait_time)});
    edge_id_to_type_[id] = WaitEdgeInfo{stop_ptr->name,static_cast<double>(settings_.bus_wait_time)};
  }
}

void TransportRouter::AddBusEdges(const Catalogue &transport_catalogue) {
  using namespace graph;

  for (const auto& bus : transport_catalogue.GetBuses()) {
    ParseBusRouteToEdges(transport_catalogue, bus);
  }
}

void TransportRouter::ParseBusRouteToEdges(const Catalogue &transport_catalogue, const Bus &bus) {

  //добавляем как ребра дистанции от начала маршрута до каждой остановки.

  if(bus.is_roundtrip){
    FillTransportRouter(bus.stops.begin(),bus.stops.end(),transport_catalogue,bus);


  }else{
    const auto end_it = std::next(bus.stops.begin(),bus.stops.size()/2);

    FillTransportRouter(bus.stops.begin(),end_it + 1,transport_catalogue,bus);

    FillTransportRouter(end_it,bus.stops.end(),transport_catalogue,bus);
  }

}

graph::Edge<double> TransportRouter::MakeBusEdge(const Stop *from, const Stop *to, const double distance) const {

  graph::Edge<double> result;
  result.from = stop_ptr_to_pair_id_.at(from).bus_wait_end;
  result.to = stop_ptr_to_pair_id_.at(to).bus_wait_begin;
  result.weight = distance * 1.0 / (settings_.bus_velocity * 1000 / 60); // перевод скорости в м/мин
  return result;
}


}
}
