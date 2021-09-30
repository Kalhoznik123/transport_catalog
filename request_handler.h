#pragma once
#include <optional>
#include <unordered_map>
#include "map_renderer.h"
#include "transport_router.h"
#include <deque>
namespace transport {

class Catalogue;
struct BusInformation;
struct StopInformation;




class RequestHandler {
public:
  // MapRenderer понадобится в следующей части итогового проекта
  RequestHandler(const transport::Catalogue& db, const renderer::MapRenderer& renderer, const router::TransportRouter& router);

  // Возвращает информацию о маршруте (запрос Bus)
  std::optional<transport::BusInformation> GetBusStat(const std::string_view& bus_name) const;

  // Возвращает маршруты, проходящие через
  std::optional<transport::StopInformation> GetBusesByStop(const std::string_view& stop_name) const;

  // Этот метод будет нужен в следующей части итогового проекта
  void RenderMap(svg::Document& doc) const;

  std::optional<router::RouteInfo> GetRouteInfo( std::string_view from, std::string_view to) const;

  const std::deque<Stop>& GetStops()const;

  const std::deque<Bus>& GetBuses()const;

  const int GetTransportCatalogStopIndex(const Stop& stop) const;

  const std::unordered_map<std::pair<const Stop*, const Stop*>, double, StopsPairHasher>& GetDistancesBetweenStops() const;

  const  renderer::RenderSettings& GetRendererSettings()const;

  const std::vector<graph::Edge<double>>& GetRouterGrafEages() const;

  const std::vector<std::vector<graph::EdgeId>>& GetRouterGrafIncidenceLists() const;

  const router::RoutingSettings GetRoutingSettings() const;

  const std::unordered_map<const Stop*, router::StopPairVertexId>& GetRouterStopToPairID()const;

  const std::unordered_map<graph::EdgeId, router::EdgeInfo>& GetRouterEdgeIdToType() const;

  const Stop& GetStopByIndex(int index) const;
private:
  // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
  const transport::Catalogue& db_;
  const renderer::MapRenderer& renderer_;
  const router::TransportRouter& router_;

};
}
