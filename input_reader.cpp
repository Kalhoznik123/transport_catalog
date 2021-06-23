#include "input_reader.h"

namespace transport_catalogue {

namespace input_reader{



std::string ReadLine(){
  std::string s ;
  std::getline(std::cin,s);
  return  s;
}

int ReadLineWithNumber() {
  int result;
  std::cin >> result;
  ReadLine();
  return result;
}


std::vector<std::string> GetRawData(char spliter, size_t initial_pos, std::string_view str){
  int64_t pos = initial_pos;
  const int64_t pos_end = str.npos;
  std::vector<std::string> result;

  while (true) {
    int64_t space = str.find(spliter, pos);

    std::string_view temp = space == pos_end ? str.substr(pos) : str.substr(pos, space - pos);

    result.push_back(std::string(RemoveSpaces(temp)));

    if (space == pos_end) {
      break;
    } else {
      pos = space + 1;
    }
  }
  return result;
}

std::string_view RemoveSpaces(std::string_view str){
  std::string_view temp ;
  if(str[0] == ' '&& str[str.size() - 1] == ' ' ){
    temp = str.substr(1,str.npos);
    temp = temp.substr(0,temp.size()-1);
  }else if(str[str.size() - 1] == ' '){
    temp = str.substr(0,str.size()-1);
  }else if(str[0] == ' '){
    temp = str.substr(1,str.npos);
  }

  return temp;
}

Queres ReadQueres(size_t count){
  Queres result;
  for (size_t i = 0; i < count;++i){
    std::string s = ReadLine() ;

    if(s[0] == 'S'){
      result.stop_queres_.push_back(std::move(s));
    }else{
      result.bus_queres_.push_back(std::move(s)) ;
    }
  }
  return result;

}

Query ParseQuery(std::string_view str){
  Query query;
  char spliter = ',';

  auto space = str.find(' ');
  ++space;
  auto double_dot_pos = str.find(':',space);

  query.name= str.substr(space,double_dot_pos - space);

  if(str[0] != 'S'){
    spliter = str.find('>',0) == str.npos ? '-':'>';
    query.is_line = spliter == '-';
    query.values = GetRawData(spliter,double_dot_pos + 1,str);
  }else{

    auto [coordinates,distances_to_stops] = GetStopData(GetRawData(spliter,double_dot_pos + 1,str));
    query.values = std::move(coordinates);
    query.stops_to_distance = std::move(distances_to_stops);

  }

  return query;
}

void Fill(Queres queres, TransportCatalogue &catalog){

  if(!queres.stop_queres_.empty()){

    using  DistanceToNeibhor = std::pair<std::string, int>;

    std::vector<std::pair<std::string,std::vector<DistanceToNeibhor>>> stops_to_naibhor_stop_distance;
    for(auto& query: queres.stop_queres_){
      Query q = ParseQuery(query);
      stops_to_naibhor_stop_distance.emplace_back(q.name,std::move(q.stops_to_distance));
      assert(q.values.size() == 2);
      catalog.AddStop(ConstractStop(q.name,std::stod(q.values[0]),std::stod(q.values[1])));

    }
    for(const auto& [stop_name,distance_to_neibhor]:stops_to_naibhor_stop_distance){
      for(const auto& [neibhor_stop_name,distance]: distance_to_neibhor){
        catalog.SetDistaceBetweenStops(stop_name,neibhor_stop_name,distance);
      }
    }
  }
  if(!queres.bus_queres_.empty()){
    for(auto& query: queres.bus_queres_){
      Query q = ParseQuery(query);
      Bus bus = ConstractBus(catalog,q.name,q.values,q.is_line);
      catalog.AddBus(std::move(bus));
    }
  }
}

std::tuple<std::vector<std::string>, std::vector<std::pair<std::string, int> > > GetStopData(std::vector<std::string> raw_data){
  using  DistanceToNeibhor = std::pair<std::string, int>;

  std::vector<std::string> coordinates;
  std::vector<DistanceToNeibhor> distace_to_stop;

  for(const auto& data:raw_data){
    const std::string templ = " to ";

    const  size_t templ_pos = data.find(templ);

    if(templ_pos !=data.npos){
      std::string stop = data.substr(templ_pos+templ.size());
      std::string distance = data.substr(0,data.find(' '));
      int dis = std::stoi(distance.substr(0,distance.size() - 1));
      distace_to_stop.emplace_back(std::move(stop),dis);
    }else{
      coordinates.push_back(std::move(data));
    }
  }
  return std::make_tuple(coordinates,distace_to_stop);
}

Bus ConstractBus(const TransportCatalogue &catalog, std::string_view bus_number, const std::vector<std::string> &values, bool is_line)  {
  Bus bus;
  bus.number = std::move(bus_number);
  double georafical_distance = 0;
  double route_length =0;

  std::unordered_set<std::string_view> uniq;

  if(is_line){
    auto prev = catalog.GetStop(values[0]);
    for(size_t i =0;i< values.size();++i ){
      if(i == 0){
        uniq.insert(values[i]);
        bus.stops_name.emplace_back(std::move(values[i]));
        continue;
      }
      auto curr = catalog.GetStop(values[i]);
      georafical_distance+= ComputeDistance(prev->coordinates,curr->coordinates)*2;
      route_length+= catalog.GetDistanceBetweenStops(prev->name,curr->name) + catalog.GetDistanceBetweenStops(curr->name,prev->name);
      uniq.insert(values[i]);
      bus.stops_name.emplace_back(std::move(values[i]));
      prev = curr;
    }
    bus.all_stops_count = ( values.size() + values.size() ) -1;
  }else{
    auto prev = catalog.GetStop(values[0]);
    for(size_t i =0;i< values.size();++i ){
      if(i == 0){
        uniq.insert(values[i]);
        bus.stops_name.emplace_back(std::move(values[i]));
        continue;
      }
      auto curr = catalog.GetStop(values[i]);
      georafical_distance+= ComputeDistance(prev->coordinates,curr->coordinates);
      route_length+= catalog.GetDistanceBetweenStops(prev->name,curr->name);
      uniq.insert(values[i]);
      bus.stops_name.emplace_back(std::move(values[i]));
      prev = curr;
    }
    bus.all_stops_count = values.size();
  }
  bus.uniqe_stops_count = uniq.size();
  bus.route_length = route_length;
  bus.curvature = bus.route_length/georafical_distance;
  return bus;
}

Stop ConstractStop(std::string_view stop_name, double lat, double lng){

  return {std::string(stop_name),{lat,lng}};
}

}
}
