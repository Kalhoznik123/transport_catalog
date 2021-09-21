#pragma once
#include <filesystem>
#include <variant>
#include <string>
#include "transport_catalogue.h"
#include<transport_catalogue.pb.h>
#include "request_handler.h"
#include "map_renderer.h"
#include <unordered_map>
#include <optional>
#include <variant>
namespace transport {

namespace serialization {



class Serializator{
public:
    template<class T>
    void SetPath(T file){

        file_name_ = std::move(file);
    }

    void SerizalizeCatalog(const RequestHandler& rh );

    void DeserializeCatalog(transport::Catalogue& catalog,renderer::MapRenderer& renderer);

    std::optional<renderer::RenderSettings> DeserializeRendererSettings(const proto_catalogue_serialization::TransportCatalogue& catalog);
private:

    std::filesystem::path file_name_;
    svg::Point DeserializedPoint(const proto_catalogue_serialization::Point& proto_point);
    svg::Color DeserializeColor(proto_catalogue_serialization::Color color_);

    proto_catalogue_serialization::Point MakeProtoPoint(int x,int y);

    proto_catalogue_serialization::Color MakeProtoColor(const svg::Color& color);
    void SerializeStopsAndFillMap(proto_catalogue_serialization::TransportCatalogue& catalog,
                                  const transport::RequestHandler& rh,
                                  std::unordered_map<std::string,int>& stops_index);
    void SerializeBuses(proto_catalogue_serialization::TransportCatalogue& catalog,
                        const RequestHandler& rh,
                        const std::unordered_map<std::string,int>& stops_index);

    void SerializeDistancesBetweenStops(proto_catalogue_serialization::TransportCatalogue& catalog,
                                        const RequestHandler& rh,
                                        const std::unordered_map<std::string,int>& stops_index);

    void SerializeRenderSettings(proto_catalogue_serialization::TransportCatalogue& catalog,
                                 const RequestHandler& rh );


    void DeserializeStops(transport::Catalogue& catalog,const proto_catalogue_serialization::TransportCatalogue& proto_catalog);

    void DeserializeDistance(transport::Catalogue& catalog,const proto_catalogue_serialization::TransportCatalogue& proto_catalog);

    void DeserializeBus(transport::Catalogue& catalog,const proto_catalogue_serialization::TransportCatalogue& proto_catalog);

};


}
}
