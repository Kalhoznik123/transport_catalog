// напишите решение с нуля
// код сохраните в свой git-репозиторий
#pragma once
#include <string>
#include <vector>
#include <list>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <functional>
#include <set>
#include "geo.h"
#include <iostream>
#include "domain.h"

namespace transport {


template <typename DomainPtr>
void SortByName(std::deque<DomainPtr>& container) {
    sort(container.begin(), container.end(),
            [](const DomainPtr& lhs, const DomainPtr& rhs) {
                return lhs->name < rhs->name;
            }
    );
}


class Catalogue {
public:

    // добавление маршрута в базу
  void AddBus(transport::Bus bus);

  // добавление остановки в базу
  void AddStop(transport::Stop stop);

  //получение информации о маршруте
  BusInformation GetBusInformation(const transport::Bus* bus) const;

  // получение списка автобусов
  stopInfoStruct GetStopInformation(const transport::Stop* stop) const;

  void SetDistanceBeetweenStops(const std::string_view& from, const std::string_view& to, double distance);

  // поиск маршрута по имени
  const Bus* GetBus(const std::string_view& num) const;

  // поиск остановки по имени
  const Stop* GetStop(const std::string_view& name) const;

  std::deque<const transport::Stop*> GetAllStopsSortedByName() const;

  std::vector<geo::Coordinates> GetAllPoints() const;

  std::deque<const transport::Bus*> GetAllBusesSortedByName() const;

private:
  struct StopsPairHasher {
    size_t operator()(const std::pair<const transport::Stop*, const transport::Stop*>& pair) const;
  private:
        std::hash<const void*> h1;
    };
    // получить расстояние между остановками
    double GetDistanceBetweenStops(const Stop* from, const Stop* to) const;

    std::unordered_map<std::string, const Stop*, std::hash<std::string>> name_to_stop_;
    std::unordered_map<std::string, const Bus*, std::hash<std::string>> name_to_bus;
    std::unordered_map<const Stop*, std::set<const transport::Bus*>> stop_to_buses_;
    std::unordered_map<std::pair<const Stop*, const Stop*>, double, StopsPairHasher> stop_pair_to_distance_;

    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
};

} // namespace transport
