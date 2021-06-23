#pragma once
#include <string>
#include <iostream>
#include <string_view>
#include <iomanip>
#include "transport_catalogue.h"

namespace  transport_catalogue {

namespace stat_reader {

enum  class QueryType{
  STOP,
  BUS
};

struct Query{
  QueryType type;
  std::string item_name;
};


std::string ReadLine();

int ReadLineWithNumber();

Query ParseQuery(std::string_view raw_query);

void PrintBusInformation(const TransportCatalogue& catalog, std::string_view bus_number);

void PrintStopInformation(const TransportCatalogue& catalog, std::string_view stop_name);

}
}
