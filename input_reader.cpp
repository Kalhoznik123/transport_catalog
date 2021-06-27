#include "input_reader.h"
#include <iostream>

namespace transport_catalogue {

namespace input_reader{



std::string ReadLine(std::istream& stream){
  std::string s ;
  std::getline(stream,s);
  return  s;
}

int ReadLineWithNumber() {
  int result;
  std::cin >> result;
  ReadLine(std::cin);
  return result;
}


std::vector<std::string> SplitStringWithValues(char spliter, size_t initial_pos, std::string_view str){
  size_t pos = initial_pos;

  std::vector<std::string> result;

  while (true) {
    size_t space = str.find(spliter, pos);

    const std::string_view temp = space == std::string::npos ? str.substr(pos) : str.substr(pos, space - pos);

    result.push_back(std::string(RemoveStartEndSpaces(temp)));

    if (space == std::string::npos) {
      break;
    } else {
      pos = space + 1;
    }
  }
  return result;
}

std::string_view RemoveStartEndSpaces(std::string_view str){
  if(str.empty()){
    static std::string_view empty("");
    return empty;
  }

  const auto first_letter_pos = str.find_first_not_of(" ");
  const auto last_letter_pos = str.find_last_not_of(" ");
  std::string_view temp  = str;

  if(last_letter_pos!= temp.npos ){
    auto aaa = temp.size() - last_letter_pos;
    temp.remove_suffix(aaa - 1);
  }
  if(first_letter_pos != 0)
    temp.remove_prefix(first_letter_pos);

  return temp;
}



Queres ReadQueres(size_t count, std::istream& stream){
  Queres result;
  for (size_t i = 0; i < count;++i){
    std::string s = ReadLine(stream) ;

    size_t space_pos = s.find(' ');

    auto first_query_word = s.substr(0,space_pos);
    if(first_query_word == "Stop"){
      result.stop_queres_.push_back(std::move(s));
    }else if(first_query_word == "Bus" ){
      result.bus_queres_.push_back(std::move(s)) ;
    }
  }
  return result;

}

std::variant<BusQuery,StopQuery> ParseQuery(std::string_view str){
  std::variant<BusQuery,StopQuery> result;

  auto space = str.find(' ');
  std::string_view type = str.substr(0,space);
  ++space;
  const auto colon_pos = str.find(':',space);

  if(type == "Bus"){
    BusQuery bus_query;

    bus_query.name= str.substr(space,colon_pos - space);
    char splitter  = str.find('>',0) == str.npos ? '-':'>';

    bus_query.is_direct_route = splitter  == '-';
    bus_query.stops = SplitStringWithValues(splitter ,colon_pos + 1,str);
    result = bus_query;
  }else if(type == "Stop") {
    StopQuery stop_query;
    stop_query.name= str.substr(space,colon_pos - space);
    char splitter  = ',';

    auto values = SplitStringWithValues(splitter ,colon_pos + 1,str);
    stop_query.lat = std::stod(values[0]);
    stop_query.lng = std::stod(values[1]);

    using  DistanceToNeibhor = std::pair<std::string, int>;
    std::vector<DistanceToNeibhor> distace_to_stop;
    const std::string templ = " to ";
    for(size_t i = 2; i<values.size();++i){
      const  size_t templ_pos = values[i].find(templ);
      std::string stop = values[i].substr(templ_pos+templ.size());
      const std::string distance = values[i].substr(0,values[i].find(' '));
      const int dis = std::stoi(distance.substr(0,distance.size() - 1));
      stop_query.stops_to_distance.emplace_back(std::move(stop),dis);
    }
    result = stop_query;
  }
  return result;
}


void Fill(Queres queres, TransportCatalogue &catalog){

  if(!queres.stop_queres_.empty()){

    using  DistanceToNeibhor = std::pair<std::string, int>;

    std::vector<std::pair<std::string,std::vector<DistanceToNeibhor>>> stops_to_neighbour_stop_distance;
    for(const auto& query: queres.stop_queres_){
      StopQuery q = std::get<StopQuery>(ParseQuery(query));

      stops_to_neighbour_stop_distance.emplace_back(q.name,std::move(q.stops_to_distance));
      catalog.AddStop(CreateStop(q.name,q.lat,q.lng));

    }
    for(const auto& [stop_name,distance_to_neighbour]:stops_to_neighbour_stop_distance){
      for(const auto& [neighbour_stop_name,distance]: distance_to_neighbour){
        catalog.SetDistaceBetweenStops(catalog.GetStop(stop_name),catalog.GetStop(neighbour_stop_name),distance);
      }
    }
  }
  if(!queres.bus_queres_.empty()){
    for(const auto& query: queres.bus_queres_){
      BusQuery q =std::get<BusQuery>(ParseQuery(query));
      Bus bus = CreateBus(catalog,q.name,q.stops,q.is_direct_route);
      catalog.AddBus(std::move(bus));
    }
  }
}


Bus CreateBus(const TransportCatalogue &catalog, std::string_view bus_number, const std::vector<std::string> &stops, bool is_line)  {
  Bus bus;
  bus.number = std::move(bus_number);
  double geographical_distance = 0;
  double route_length =0;

  std::unordered_set<std::string_view> unique_stops;

  if(is_line){
    auto prev = catalog.GetStop(stops[0]);
    for(size_t i =0;i< stops.size();++i ){
      if(i == 0){
        unique_stops.insert(stops[i]);
        bus.stops_name.emplace_back(std::move(stops[i]));
        continue;
      }
      const auto curr = catalog.GetStop(stops[i]);
      geographical_distance+= ComputeDistance(prev->coordinates,curr->coordinates)*2;
      route_length+= catalog.GetDistanceBetweenStops(prev,curr) + catalog.GetDistanceBetweenStops(curr,prev);
      unique_stops.insert(stops[i]);
      bus.stops_name.emplace_back(std::move(stops[i]));
      prev = curr;
    }
    bus.all_stops_count = ( stops.size() + stops.size() ) -1;
  }else{
    auto prev = catalog.GetStop(stops[0]);
    for(size_t i =0;i< stops.size();++i ){
      if(i == 0){
        unique_stops.insert(stops[i]);
        bus.stops_name.emplace_back(std::move(stops[i]));
        continue;
      }
      const auto curr = catalog.GetStop(stops[i]);
      geographical_distance+= ComputeDistance(prev->coordinates,curr->coordinates);
      route_length+= catalog.GetDistanceBetweenStops(prev,curr);
      unique_stops.insert(stops[i]);
      bus.stops_name.emplace_back(std::move(stops[i]));
      prev = curr;
    }
    bus.all_stops_count = stops.size();
  }
  bus.unique_stops_count = unique_stops.size();
  bus.route_length = route_length;
  bus.curvature = bus.route_length/geographical_distance;
  return bus;
}

Stop CreateStop(std::string_view stop_name, double lat, double lng){

  return {std::string(stop_name),{lat,lng}};
}

}
}
