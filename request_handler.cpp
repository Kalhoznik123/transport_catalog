#include "request_handler.h"

RequestHandler::RequestHandler(const transport::Catalogue& db, const renderer::MapRenderer& renderer) : db_(db), renderer_(renderer) {

}

std::optional<transport::BusInformation> RequestHandler::GetBusStat(const std::string_view& bus_name) const {

    auto bus = db_.GetBus(bus_name);

    if (bus == nullptr) {
        return std::nullopt;
    }else{
        return db_.GetBusInformation(std::move(bus));
    }
}

std::optional<transport::stopInfoStruct> RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {

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
