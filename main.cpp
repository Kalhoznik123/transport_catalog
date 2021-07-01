#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"
#include <iostream>

using namespace std;
using namespace transport_catalogue;



int ReadLineWithNumber(std::istream& stream ) {
  int result;
  std::cin >> result;
  input_reader::ReadLine(stream);
  return result;
}



int main(){

  TransportCatalogue catalog;

  int query_count = ReadLineWithNumber(std::cin);

  Fill(input_reader::ReadQueres(query_count,std::cin),catalog);

  /// ниже выполняются запросы на вывод

  int bus_query_count = ReadLineWithNumber(std::cin);

  for(int i  = 0;i<bus_query_count;++i){
    const stat_reader::Query query = stat_reader::ParseQuery(stat_reader::ReadLine());

    switch (query.type) {
    case stat_reader::QueryType::STOP:
      stat_reader::PrintStopInformation(catalog,query.item_name,std::cout);
      break;
    case stat_reader::QueryType::BUS:
      stat_reader::PrintBusInformation(catalog,query.item_name,std::cout);
      break;
    }
  }

}
