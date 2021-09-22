#pragma once
#include <filesystem>
#include <variant>
#include <string>
#include "transport_catalogue.h"
#include<transport_catalogue.pb.h>
#include "request_handler.h"
#include "map_renderer.h"
#include <unordered_map>
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

private:

    std::filesystem::path file_name_;

    renderer::RenderSettings DeserializeRendererSettings(const proto_catalogue_serialization::TransportCatalogue& catalog);

    svg::Point DeserializePoint(const proto_catalogue_serialization::Point& proto_point);
    svg::Color DeserializeColor(const proto_catalogue_serialization::Color& proto_color);

    proto_catalogue_serialization::Point SerializedPoint(const svg::Point& point);
    proto_catalogue_serialization::Color SerializedColor(const svg::Color& color);
proto_catalogue_serialization::RGB_color SerializedRGB(const svg::Rgb& rgba);
    proto_catalogue_serialization::RGBA_color SerializedRGBA(const svg::Rgba& rgb);
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

