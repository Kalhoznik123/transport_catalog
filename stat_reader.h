#pragma once
#include <string>
#include <iostream>
#include <string_view>
#include <iomanip>
#include "transport_catalogue.h"

namespace  transport_catalogue {

namespace stat_reader {


std::string ReadLine();

int ReadLineWithNumber();

std::string_view GetNumberFromQuery(std::string_view query);

void PrintBusInformation(const TransportCatalogue& catalog, std::string_view bus_number);

void PrintStopInformation(const TransportCatalogue& catalog, std::string_view stop_name);

}
}
