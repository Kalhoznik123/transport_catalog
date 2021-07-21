#pragma once
#include <string>
#include <vector>
#include <unordered_set>
#include "geo.h"

namespace transport {

struct Stop {
    std::string name;
    geo::Coordinates coordinate;
};

struct Bus {
    std::string name;
    std::vector<const Stop*> stops;
    const Stop* start_stop;
    const Stop* end_stop;

    bool operator<(const Bus& rhs) const;
};

struct BusInformation {
    int stops =0;
    int unique_stops =0;
    double route_length = 0;
    double curvature = 0;
};

struct StopInformation {

    std::unordered_set<const Bus*> buses;
};

} // namespace transport
