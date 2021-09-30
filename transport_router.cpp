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

void TransportRouter::BuildRouter(std::vector<graph::Edge<double> > edges,
                                  std::vector<std::vector<graph::EdgeId> > incidence_lists,
                                  std::unordered_map<const Stop*, StopPairVertexId> stop_ptr_to_pair_id,
                                  std::unordered_map<graph::EdgeId, EdgeInfo> edge_id_to_type){

    SetStopPtdToPairId(std::move(stop_ptr_to_pair_id));
    SetEdgeIdToType(std::move(edge_id_to_type));
    router_graf_ = std::make_unique<graph::DirectedWeightedGraph<double>>(std::move(edges),std::move(incidence_lists));
    router_ = std::make_unique<graph::Router<double>>(*router_graf_);
}

std::optional<StopPairVertexId> TransportRouter::GetPairVertexId(const Stop *stop) const{
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

const std::vector<graph::Edge<double> > &TransportRouter::GetGrafEdges() const{
    return router_graf_->GetEdges();
}

const std::vector<std::vector<graph::EdgeId> >& TransportRouter::GetGrafIncidenceLists() const{
    return router_graf_->GetIncidenceList();
}

const std::unordered_map<const Stop *, StopPairVertexId>& TransportRouter::GetStopToPairID() const{
    return stop_ptr_to_pair_id_;
}

const std::unordered_map<graph::EdgeId, EdgeInfo>& TransportRouter::GetEdgeIdToType() const{
    return edge_id_to_type_;
}

const RoutingSettings &TransportRouter::GetRoutingSettings() const{
    return settings_;
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

void TransportRouter::AddToStopPtrPairId(const Stop *key, const StopPairVertexId &value){
    stop_ptr_to_pair_id_[key] = value;
}

void TransportRouter::AddToEdgeIdToType(graph::EdgeId key, const EdgeInfo &value){
    edge_id_to_type_[key] = value;
}

void TransportRouter::SetStopPtdToPairId(std::unordered_map<const Stop *, StopPairVertexId> stop_ptr_to_pair_id){
    stop_ptr_to_pair_id_ = std::move(stop_ptr_to_pair_id);
}

void TransportRouter::SetEdgeIdToType(std::unordered_map<graph::EdgeId, EdgeInfo> edge_id_to_type){
    edge_id_to_type_ = std::move(edge_id_to_type);
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
