// напишите решение с нуля
// код сохраните в свой git-репозиторий
#include "transport_catalogue.h"
#include <algorithm>



template <typename DomainPtr>
void SortByName(std::vector<DomainPtr>& container) {
  sort(container.begin(), container.end(),
       [](const DomainPtr& lhs, const DomainPtr& rhs) {
         return lhs->name < rhs->name;
       }
       );
}


size_t transport::Catalogue::StopsPairHasher::operator()(const std::pair<const transport::Stop*, const transport::Stop*>& pair) const {
  const size_t h_1 = h1(pair.first);
   const  size_t h_2 = h1(pair.second);
    return h_2 + h_1 * 13;
}


void  transport::Catalogue::AddBus(transport::Bus bus) {

    buses_.push_back(std::move(bus));

    Bus& bus_reference  = buses_.back();

    name_to_bus[bus_reference.name] = &bus_reference;

    for (const Stop* stop : bus_reference.stops) {
        stop_to_buses_.at(stop).insert(&bus_reference);
    }

}


void transport::Catalogue::AddStop(transport::Stop stop) {
    stops_.push_back(std::move(stop));

    Stop& stop_reference  = stops_.back();


    name_to_stop_[stop_reference.name] = &stop_reference;
    stop_to_buses_[&stop_reference] = {};

}


const transport::Bus* transport::Catalogue::GetBus(const std::string_view& num) const {
    if(name_to_bus.count(std::string(num)) == 0)
        return  nullptr;
    return name_to_bus.at(std::string(num));
}


const transport::Stop* transport::Catalogue::GetStop(const std::string_view& name) const {
    if (name_to_stop_.count(std::string(name)) == 0) {
        return nullptr;
    }

    return name_to_stop_.at(std::string(name));
}

void transport::Catalogue::SetDistanceBeetweenStops(const std::string_view& from, const std::string_view& to, double distance) {
   stop_pair_to_distance_[{GetStop(from),GetStop(to)}] = distance;
}

double transport::Catalogue::GetDistanceBetweenStops(const Stop* from, const Stop* to) const {
 const  auto find_from_to_item = stop_pair_to_distance_.find({from, to});

  if(find_from_to_item != stop_pair_to_distance_.end()){
    return find_from_to_item->second;
  }else if(auto find_from_to_item = stop_pair_to_distance_.find({to, from}); find_from_to_item != stop_pair_to_distance_.end()){
    return find_from_to_item->second;

  }
    return 0;

}

transport::BusInformation transport::Catalogue::GetBusInformation(const transport::Bus* bus) const {

  const std::vector<const Stop*>& route = bus->stops;
    std::unordered_set<const Stop*> set_temp(route.begin(), route.end());

    double dist = 0;
    for (size_t i = 0; i < route.size() - 1; ++i) {
        dist += ComputeDistance(route.at(i)->coordinate,route.at(i+1)->coordinate);
    }

    if (dist == 0) {
        dist = 1;
    }

    double route_length = 0;
    for (size_t i = 0; i < route.size() - 1; ++i) {
        route_length += GetDistanceBetweenStops(route.at(i),route.at(i+1));
    }
    return {static_cast<int>(route.size()), static_cast<int>(set_temp.size()), route_length, route_length/dist};
}

transport::StopInformation transport::Catalogue::GetStopInformation(const transport::Stop* stop) const {
    std::unordered_set<const Bus*> buses;


    for (const auto& bus : stop_to_buses_.at(stop)) {
        buses.insert(bus);
    }
    return {buses};
}

std::vector<const transport::Stop*> transport::Catalogue::GetAllStopsSortedByName() const {

  std::vector<const transport::Stop*> stops;
  for(const auto& stop: name_to_stop_){
    if (!GetStopInformation(stop.second).buses.empty()) {
      stops.push_back(stop.second);
    }
  }
  SortByName(stops);
  return stops;

}

std::vector<geo::Coordinates> transport::Catalogue::GetAllPoints() const {
  std::vector<geo::Coordinates> points;
  for(const auto& stop: name_to_stop_){
    if (!GetStopInformation(stop.second).buses.empty()) {
      points.push_back(stop.second->coordinate);
    }
  }
  return points;
}

std::vector<const transport::Bus*> transport::Catalogue::GetAllBusesSortedByName() const {

  std::vector<const transport::Bus*> buses;
  for(const auto& bus: name_to_bus){

    buses.push_back(bus.second);
  }
  SortByName(buses);
  return buses;
}

const std::deque<transport::Stop> &transport::Catalogue::GetStops() const{
  return stops_;
}

const std::deque<transport::Bus> &transport::Catalogue::GetBuses() const{
  return  buses_;
}



