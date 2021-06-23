#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <cmath>
#include <string_view>
#include "transport_catalogue.h"
#include <utility>
#include <tuple>
#include <cassert>

namespace transport_catalogue {

namespace input_reader {


struct Query {
  std::string name;
  bool is_line = false ;
  std::vector<std::string> values;
  std::vector<std::pair<std::string,int>> stops_to_distance;
};

struct Queres{
  std::vector<std::string> stop_queres_;
  std::vector<std::string> bus_queres_;
};


std::string ReadLine();

int ReadLineWithNumber();

inline std::string_view RemoveSpaces(std::string_view str);

std::vector<std::string>GetRawData(char spliter, size_t initial_pos, std::string_view str);

std::tuple<std::vector<std::string>,std::vector<std::pair<std::string,int>>> GetStopData(std::vector<std::string> raw_data);

Queres ReadQueres(size_t count);

Query ParseQuery(std::string_view str);

void Fill(Queres queres,TransportCatalogue& catalog);

Bus ConstractBus(const TransportCatalogue& catalog, std::string_view bus_number, const std::vector<std::string>& stops_name, bool is_line);

Stop ConstractStop(std::string_view stop_name, double lat,double lng);

}
}
