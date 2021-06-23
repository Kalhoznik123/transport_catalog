#include "transport_catalogue.h"

namespace transport_catalogue {


void TransportCatalogue::AddBus(Bus bus){

  //Bus bus = MakeBus(std::move(bus_number),stops,is_line);
  const auto& inset_bus = buses_.emplace_back(std::move(bus));
  bus_name_to_bus_[inset_bus.number] = &inset_bus;

  for(const auto& stop: inset_bus.stops_name){
    stops_to_bus_[GetStop(stop)].insert(inset_bus.number);
  }
}

void TransportCatalogue::AddStop(std::string_view stop_name, double lat,double lng){
  detail::Coordinates coor{lat,lng};
  Stop stop{std::string(stop_name),coor};
  const auto& inset_stop = stops_.emplace_back(std::move(stop));
  stop_name_to_stop_[inset_stop.name] = &inset_stop;
  stops_to_bus_[&inset_stop];
}

const Bus *TransportCatalogue::GetBus(std::string_view bus_number) const {
  if(!bus_name_to_bus_.count(bus_number))
    return nullptr;
  return bus_name_to_bus_.at(bus_number);
}

const Stop *TransportCatalogue::GetStop(std::string_view stop_name) const {
  if(!stop_name_to_stop_.count(stop_name))
    return nullptr;
  return stop_name_to_stop_.at(stop_name);
}

const std::set<std::string_view> &TransportCatalogue::GetStopInformation(std::string_view stop_name) const {

  return stops_to_bus_.at(GetStop(stop_name));
}

void TransportCatalogue::SetDistaceBetweenStops(std::string_view stop_name, std::string_view neibhor_stop_name, double distace){
  stop_pair_to_distance_.insert_or_assign(std::make_pair(GetStop(stop_name),GetStop(neibhor_stop_name)),distace);
  }

double TransportCatalogue::GetDistanceBetweenStops(std::string_view stop_name, std::string_view neibhor_stop_name) const {
  auto stops = std::make_pair(GetStop(stop_name),GetStop(neibhor_stop_name));
  double result = 0;
  if(stop_pair_to_distance_.count(stops)){
    result = stop_pair_to_distance_.at(stops);
  }else{
    result = stop_pair_to_distance_.at(std::make_pair(GetStop(neibhor_stop_name),GetStop(stop_name)));
  }
  return  result;
}

}
