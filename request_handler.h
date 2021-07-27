#pragma once
#include <optional>
#include "transport_catalogue.h"
#include "map_renderer.h"

namespace transport {



class RequestHandler {
public:
  // MapRenderer понадобится в следующей части итогового проекта
  RequestHandler(const transport::Catalogue& db,const renderer::MapRenderer& renderer);

  // Возвращает информацию о маршруте (запрос Bus)
  std::optional<transport::BusInformation> GetBusStat(const std::string_view& bus_name) const;

  // Возвращает маршруты, проходящие через
  std::optional<transport::StopInformation> GetBusesByStop(const std::string_view& stop_name) const;

  // Этот метод будет нужен в следующей части итогового проекта
  void RenderMap(svg::Document& doc) const;

private:
  // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
  const transport::Catalogue& db_;
  const renderer::MapRenderer& renderer_;
};
}
