#pragma once

#include <iostream>
#include "request_handler.h"
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

namespace transport {

class Reader {
public:
  explicit Reader(transport::Catalogue& catalogue, renderer::MapRenderer& renderer);

  // парсит текст из потоков в JSON
  void FillCatalog(std::istream& in);

  void PrintRequests(std::ostream& out) const;

  static  svg::Color ParseColor(json::Node node) ;
private:

  renderer::RenderSettings ParseVisualisationSettings() const;
  // добавляет в базу остановки
  void AddStops() const;

  // добавляет в базу расстояния между остановками
  void AddDistances() const;

  // добавляет в базу маршруты
  void AddBuses() const;

  json::Node StopStat(const json::Node& node) const;

  json::Node RouteStat(const json::Node& node) const;

  json::Node MapStat(const json::Node& node) const;

  json::Document returnStat() const;


  transport::Catalogue& catalogue_;
  renderer::MapRenderer& renderer_;
  json::Array base_requests_;
  json::Array stat_requests_;
  json::Dict render_settings_;
};

} // namespace json
