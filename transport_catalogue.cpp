#include "transport_catalogue.h"

namespace transport_catalogue {


void TransportCatalogue::AddBus(Bus bus){
  buses_.push_back(std::move(bus));
  const auto& inset_bus = buses_.back();
  buses_name_to_bus_[inset_bus.number] = &inset_bus;

  for(const auto& stop: inset_bus.stops_name){
    stop_to_buses_name_[GetStop(stop)].insert(inset_bus.number);
  }
}

void TransportCatalogue::AddStop(Stop stop){
  stops_.push_back(std::move(stop));
  const auto& inset_stop = stops_.back();
  stops_name_to_stop_[inset_stop.name] = &inset_stop;
  stop_to_buses_name_[&inset_stop];
}

const Bus *TransportCatalogue::GetBus(std::string_view bus_number) const {
  const auto find_item_iter = buses_name_to_bus_.find(bus_number);

  if(find_item_iter == buses_name_to_bus_.end())
    return nullptr;
  return find_item_iter->second;
}

const Stop *TransportCatalogue::GetStop(std::string_view stop_name) const {
   const auto find_item_iter = stops_name_to_stop_.find(stop_name);

  if(find_item_iter == stops_name_to_stop_.end())
    return nullptr;
  return find_item_iter->second;
}

const std::set<std::string_view> &TransportCatalogue::GetBuses(std::string_view stop_name) const {

  return stop_to_buses_name_.at(GetStop(stop_name));
}

void TransportCatalogue::SetDistaceBetweenStops(const Stop* stop, const Stop* neighbour_stop, double distace){
  stops_pair_to_distance_.insert_or_assign(std::make_pair(stop,neighbour_stop),distace);
}

double TransportCatalogue::GetDistanceBetweenStops(const Stop* stop, const Stop* neighbour_stop) const {


  double result = 0;

  const auto find_item_iter = stops_pair_to_distance_.find(std::make_pair(stop,neighbour_stop));


  if(find_item_iter != stops_pair_to_distance_.end()){
    result = find_item_iter->second;
  }else{
    result = stops_pair_to_distance_.at(std::make_pair(neighbour_stop,stop));
  }
  return  result;
}

}
