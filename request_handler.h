#pragma once
#include <optional>
#include "map_renderer.h"
#include "transport_router.h"

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

private:
  // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
  const transport::Catalogue& db_;
  const renderer::MapRenderer& renderer_;
  const router::TransportRouter& router_;

};
}
