#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"
#include <iostream>

using namespace std;
using namespace transport_catalogue;

int main(){

  TransportCatalogue catalog;

  int query_count = input_reader::ReadLineWithNumber();

  auto query = input_reader::ReadQueres(query_count);

  Fill(std::move(query),catalog);


  /// ниже выполняются запросы на вывод
  int bus_query_count = stat_reader::ReadLineWithNumber();

  for(int i  = 0;i<bus_query_count;++i){
    const auto query = stat_reader::ReadLine();
    if(query[0] == 'S'){
      stat_reader::PrintStopInformation(catalog,stat_reader::GetNumberFromQuery(query));
    }else{
      stat_reader::PrintBusInformation(catalog,stat_reader::GetNumberFromQuery(query));
    }

  }

}
