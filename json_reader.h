#pragma once


#include "json.h"
#include "svg.h"
namespace transport {


class Catalogue;
namespace renderer  {
class MapRenderer;
struct RenderSettings;
}
namespace router {
class TransportRouter;
struct RoutingSettings;
}

class Reader {
public:
  explicit Reader(transport::Catalogue& catalogue, renderer::MapRenderer& renderer, router::TransportRouter& router);

  // парсит текст из потоков в JSON
  void ParseRequests(std::istream& in);

  void PrintReply(std::ostream& out) const;

private:
  static  svg::Color ParseColor(json::Node node) ;

  renderer::RenderSettings ParseVisualisationSettings() const;
  // добавляет в базу остановки
  void AddStops() const;

  // добавляет в базу расстояния между остановками
  void AddDistances() const;

  // добавляет в базу маршруты
  void AddBuses() const;

  json::Node StopStat(const json::Node& node) const;

  json::Node BusStat(const json::Node& node) const;

  json::Node MapStat(const json::Node& node) const;

  json::Node RouteStat(const json::Node& node) const;

  json::Document ReturnStat() const;

  router::RoutingSettings ParseRoutingSettings();


private:
  transport::Catalogue& catalogue_;
  renderer::MapRenderer& renderer_;
  router::TransportRouter& router_;

  json::Array base_requests_;
  json::Array stat_requests_;
  json::Dict render_settings_;
  json::Dict route_settings_;
};

} // namespace json
