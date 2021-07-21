// напишите решение с нуля
// код сохраните в свой git-репозиторий
#include "transport_catalogue.h"
#include <algorithm>
#include <stdexcept>
#include <iostream>

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
    auto aaa= name_to_stop_.at(std::string(name));
    return aaa;
}

void transport::Catalogue::SetDistanceBeetweenStops(const std::string_view& from, const std::string_view& to, double distance) {
   stop_pair_to_distance_[{GetStop(from),GetStop(to)}] = distance;
}

double transport::Catalogue::GetDistanceBetweenStops(const Stop* from, const Stop* to) const {
    if (stop_pair_to_distance_.count({from, to})) {
        return stop_pair_to_distance_.at({from, to});
    }
    else if (stop_pair_to_distance_.count({to, from})){
        return stop_pair_to_distance_.at({to, from});
    }
    return 0;
}

transport::BusInformation transport::Catalogue::GetBusInformation(const transport::Bus* bus) const {

  const std::vector<const Stop*>& route = bus->stops;
    std::unordered_set<const Stop*> set_temp(route.begin(), route.end());

    double dist = 0;
    for (int i = 0; i < static_cast<int>(route.size()) - 1; ++i) {
        dist += ComputeDistance(route.at(i)->coordinate,route.at(i+1)->coordinate);
    }

    if (dist == 0) {
        dist = 1;
    }

    double route_length = 0;
    for (int i = 0; i < static_cast<int>(route.size()) - 1; ++i) {
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

std::deque<const transport::Stop*> transport::Catalogue::GetAllStopsSortedByName() const {

    std::deque<const transport::Stop*> stops;
    for (auto it = name_to_stop_.begin(); it != name_to_stop_.end(); ++it) {

        if (!GetStopInformation(it->second).buses.empty()) {
            stops.push_back(it->second);
        }
        SortByName(stops);
    }
    return stops;

}

std::vector<geo::Coordinates> transport::Catalogue::GetAllPoints() const {
    std::vector<geo::Coordinates> points;
    for (auto it = name_to_stop_.begin(); it != name_to_stop_.end(); ++it) {
        if (!GetStopInformation(it->second).buses.empty()) {
            points.push_back(it->second->coordinate);
        }
    }
    return points;
}

std::deque<const transport::Bus*> transport::Catalogue::GetAllBusesSortedByName() const {

    std::deque<const transport::Bus*> buses;

    for (auto it = name_to_bus.begin(); it != name_to_bus.end(); ++it) {

        buses.push_back(it->second);
        SortByName(buses);
    }
    return buses;
}



