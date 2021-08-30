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

std::optional<router::RouteInfo> RequestHandler::GetRouteInfo( std::string_view from,  std::string_view to) const{

  const Stop* const stop_from = db_.GetStop(from);
  const Stop* const stop_to = db_.GetStop(to);


  const auto route_info =router_.GetRouteInfo(stop_from,stop_to);

  if(!route_info)
    return std::nullopt;


  return  route_info;
}
}
