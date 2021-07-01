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
  geography::Coordinates coordinates;
};

struct Bus{
  std::string number;
  double route_length = 0;
  double curvature =0;
  size_t unique_stops_count = 0;
  size_t all_stops_count = 0;
  std::vector<std::string> stops_name;
};

class TransportCatalogue{

public:

  void AddBus(Bus bus);

  void AddStop(Stop stop);

  const Bus* GetBus(std::string_view bus_number)const;

  const Stop* GetStop(std::string_view stop_name)const;

  const std::set<std::string_view>& GetBuses(std::string_view stop_name)const;

  void SetDistaceBetweenStops(const Stop* stop, const Stop* neighbour_stop,double distace );

  double GetDistanceBetweenStops(const Stop* stop, const Stop* neighbour_stop)const;

private:
  struct StopsPairHasher {
    size_t operator() (const std::pair<const Stop*, const Stop*>& pair) const {
      const size_t h_1 = stop_pointer_hasher(pair.first);
      const size_t h_2 = stop_pointer_hasher(pair.second);
      return h_2 + h_1 * (13 * 13);
    }
  private:
    std::hash<const void*> stop_pointer_hasher;
  };


  std::deque<Bus> buses_;
  std::deque<Stop> stops_;
  std::unordered_map<std::string_view,const Bus*> buses_name_to_bus_;
  std::unordered_map<std::string_view,const Stop*> stops_name_to_stop_;
  std::unordered_map<const Stop*,std::set<std::string_view>> stop_to_buses_name_;
  std::unordered_map<const std::pair<const Stop*, const Stop*>, double, StopsPairHasher> stops_pair_to_distance_;
};

}
