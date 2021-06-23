#pragma once
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <string_view>
#include <set>
#include "geo.h"

namespace transport_catalogue {


struct Stop{
  std::string name;
  detail::Coordinates coordinates;
};

struct Bus{
  std::string number;
  double route_length = 0;
  double curvature =0;
  size_t uniqe_stops_count = 0;
  size_t all_stops_count = 0;
  std::vector<std::string> stops_name;
};


class TransportCatalogue{

public:

  struct PairHasher {
    size_t operator() (const std::pair<const Stop*, const Stop*>& pair) const {
      size_t h_1 = h1(pair.first);
      size_t h_2 = h1(pair.second);
      return h_2 * 13 + h_1 * (13 * 13);
    }
  private:
    std::hash<const void*> h1;
  };



  void AddBus(Bus bus);

  void AddStop(std::string_view stop_name, double lat, double lng);

  const Bus* GetBus(std::string_view bus_number)const;

  const Stop* GetStop(std::string_view stop_name)const;

  const std::set<std::string_view>& GetStopInformation(std::string_view stop_name)const;

  void SetDistaceBetweenStops(std::string_view stop_from, std::string_view stop_to,double distace );

  double GetDistanceBetweenStops(std::string_view stop_from, std::string_view stop_to)const;


  std::pair<std::string,const Bus*> FindBus(std::string_view bus_number){
    if(!bus_name_to_bus_.count(bus_number))
      return {std::string(bus_number),nullptr};
    return {std::string(bus_number),bus_name_to_bus_.at(bus_number)};
  }

  std::pair<std::string,const Stop*> FindStop(std::string_view stop_name){
    if(!stop_name_to_stop_.count(stop_name))
      return {std::string(stop_name),nullptr};
    return {std::string(stop_name),stop_name_to_stop_.at(stop_name)};
  }
private:

  std::deque<Bus> buses_;
  std::deque<Stop> stops_;
  std::unordered_map<std::string_view,const Bus*> bus_name_to_bus_;
  std::unordered_map<std::string_view,const Stop*> stop_name_to_stop_;
  std::unordered_map<const Stop*,std::set<std::string_view>> stops_to_bus_;
  std::unordered_map<const std::pair<const Stop*, const Stop*>, double, PairHasher> stop_pair_to_distance_;
};

}
