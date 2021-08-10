#include "request_handler.h"

namespace transport {


RequestHandler::RequestHandler(const transport::Catalogue& db, const transport::renderer::MapRenderer& renderer, const transport::router::TransportRouter& router )
    : db_(db)
    , renderer_(renderer)
    , router_(router){

}

std::optional<transport::BusInformation> RequestHandler::GetBusStat(const std::string_view& bus_name) const {

    auto bus = db_.GetBus(bus_name);

    if (bus == nullptr) {
        return std::nullopt;
    }else{
        return db_.GetBusInformation(std::move(bus));
    }
}

std::optional<transport::StopInformation> RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {

    auto stop = db_.GetStop(stop_name);

    if (stop == nullptr) {
        return std::nullopt;
    }else{
        return db_.GetStopInformation(std::move(stop));
    }
}

void RequestHandler::RenderMap(svg::Document& doc) const {

    renderer_.Render(doc,db_.GetAllStopsSortedByName(),db_.GetAllBusesSortedByName(),db_.GetAllPoints());
}

std::optional<router::RouteInfo> RequestHandler::GetRouteInfo(const std::string from, const std::string to) const{

  const Stop* stop_from = db_.GetStop(from);
  const Stop* stop_to = db_.GetStop(to);

  const auto from_id = router_.GetPairVertexId(stop_from);
  const auto to_id = router_.GetPairVertexId(stop_to);

  if(!from_id || !to_id)
    return std::nullopt;

  const auto route_info =router_.GetRouteInfo(from_id->bus_wait_begin,to_id->bus_wait_begin);

  if(!route_info)
    return std::nullopt;


  return  route_info;
}
}
