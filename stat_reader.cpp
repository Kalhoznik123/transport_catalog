#include "stat_reader.h"

namespace transport_catalogue {

namespace stat_reader {

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

std::string_view GetNumberFromQuery(std::string_view query){
  size_t space  = query.find(' ',0);
  query.remove_prefix(space + 1);
  return query;
}

void PrintBusInformation(const TransportCatalogue &catalog, std::string_view bus_number){
  auto bus = catalog.GetBus(bus_number);
  if(bus == nullptr){
    std::cout<<"Bus " << bus_number<<": not found"<<std::endl;
  }else{
    std::cout<<"Bus " <<bus->number<<": "<<bus->all_stops_count<<" stops on route, " <<bus->uniqe_stops_count<<" unique stops, " <<std::setprecision(6)
              << bus->route_length<<" route length, " << bus->curvature << " curvature" <<std::endl;
  }

}

void PrintStopInformation(const TransportCatalogue &catalog, std::string_view stop_name){

  auto stop = catalog.GetStop(stop_name);
  if(stop == nullptr){
    std::cout<<"Stop " << stop_name<<": not found"<<std::endl;
  }else{
    const auto& buses = catalog.GetStopInformation(stop->name);
    if(buses.empty()){
      std::cout<< "Stop " << stop_name<<": no buses"<<std::endl;
    }else{
      std::cout<< "Stop " << stop_name<<": buses";
      for(const auto& bus : buses){
        std::cout <<' '<<bus;
      }
      std::cout<<std::endl;
    }

  }
}


}

}
