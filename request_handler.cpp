#include "request_handler.h"

namespace transport {


RequestHandler::RequestHandler(const transport::Catalogue& db, const transport::renderer::MapRenderer& renderer, const transport::router::TransportRouter& router)
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

const std::deque<Stop> &RequestHandler::GetStops() const {
    return  db_.GetStops();
}

const std::deque<Bus> &RequestHandler::GetBuses() const {
    return  db_.GetBuses();
}

const int RequestHandler::GetTransportCatalogStopIndex(const Stop &stop) const{
return db_.GetStopIndex(stop);
}

const std::unordered_map<std::pair<const Stop *, const Stop *>, double, StopsPairHasher> &RequestHandler::GetDistancesBetweenStops() const
{
    return db_.GetDistancesBetweenStops();
}

const renderer::RenderSettings &RequestHandler::GetRendererSettings() const {
    return renderer_.GetRenderSetting();
}

const std::vector<graph::Edge<double> >& RequestHandler::GetRouterGrafEages() const {
    return router_.GetGrafEdges();
}

const router::RoutingSettings RequestHandler::GetRoutingSettings() const{
    return router_.GetRoutingSettings();
}

const std::unordered_map<const Stop *, router::StopPairVertexId> &RequestHandler::GetRouterStopToPairID() const
{
    return router_.GetStopToPairID();
}

const std::unordered_map<graph::EdgeId, router::EdgeInfo> &RequestHandler::GetRouterEdgeIdToType() const{
    return router_.GetEdgeIdToType();
}

const Stop &RequestHandler::GetStopByIndex(int index) const{
    return db_.GetStopByIndex(index);
}

const std::vector<std::vector<graph::EdgeId> > &RequestHandler::GetRouterGrafIncidenceLists() const{
    return router_.GetGrafIncidenceLists();
}



}
