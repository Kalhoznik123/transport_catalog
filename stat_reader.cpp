#include <iomanip>
#include "stat_reader.h"

namespace transport_catalogue {

namespace stat_reader {

std::string ReadLine(){
  std::string s ;
  std::getline(std::cin,s);
  return  s;
}



Query ParseQuery(std::string_view raw_query){

  stat_reader::Query query;
  size_t space_pos  = raw_query.find(' ',0);

  if(raw_query.substr(0,space_pos) == "Stop"){
    query.type = QueryType::STOP;

  }else if(raw_query.substr(0,space_pos) == "Bus")
    query.type = QueryType::BUS;

  query.item_name = raw_query.substr(space_pos + 1,std::string::npos);
  return query;
}

void PrintBusInformation(const TransportCatalogue &catalog, std::string_view bus_number,std::ostream& stream){
 const auto bus = catalog.GetBus(bus_number);
  if(bus == nullptr){
    stream<<"Bus " << bus_number<<": not found"<<std::endl;
  }else{
    stream<<"Bus " <<bus->number<<": "<<bus->all_stops_count<<" stops on route, " <<bus->unique_stops_count<<" unique stops, " <<std::setprecision(6)
              << bus->route_length<<" route length, " << bus->curvature << " curvature" <<std::endl;
  }
}

void PrintStopInformation(const TransportCatalogue &catalog, std::string_view stop_name,std::ostream& stream){

  auto stop = catalog.GetStop(stop_name);
  if(stop == nullptr){
    stream<<"Stop " << stop_name<<": not found"<<std::endl;
  }else{
    const auto& buses = catalog.GetBusesToStop(stop->name);
    if(buses.empty()){
      stream<< "Stop " << stop_name<<": no buses"<<std::endl;
    }else{
      stream<< "Stop " << stop_name<<": buses";
      for(const auto& bus : buses){
        stream <<' '<<bus;
      }
     stream<<std::endl;
    }

  }
}
}
}
