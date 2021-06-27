#pragma once
#include <string>
#include <vector>
#include <string_view>
#include <utility>
#include <tuple>
#include <variant>
#include <iostream>
#include "transport_catalogue.h"

namespace transport_catalogue {

namespace input_reader {


struct Query {
  std::string name;
  bool is_direct_route = false ;
  std::vector<std::string> values;
  std::vector<std::pair<std::string,int>> stops_to_distance;
};


struct BusQuery{
  std::string name;
  bool is_direct_route = false ;
  std::vector<std::string> stops;
};

struct StopQuery{
  std::string name;
  double lat;
  double lng;
  std::vector<std::pair<std::string,int>> stops_to_distance;
};

struct Queres{
  std::vector<std::string> stop_queres_;
  std::vector<std::string> bus_queres_;
};

std::string ReadLine(std::istream& stream);

int ReadLineWithNumber();

std::string_view RemoveStartEndSpaces(std::string_view str);

std::vector<std::string> SplitStringWithValues(char spliter, size_t initial_pos, std::string_view str);

Queres ReadQueres(size_t count,std::istream& stream);

std::variant<BusQuery,StopQuery> ParseQuery(std::string_view str);


void Fill(Queres queres,TransportCatalogue& catalog);

Bus CreateBus(const TransportCatalogue& catalog, std::string_view bus_number, const std::vector<std::string>& stops_name, bool is_line);

Stop CreateStop(std::string_view stop_name, double lat,double lng);

}
}
