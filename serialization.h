#pragma once
#include <filesystem>
#include <variant>
#include <string>
#include <transport_catalogue.pb.h>
#include <unordered_map>
#include <variant>
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"
namespace transport {

namespace serialization {



class Serializator{
public:
    template<class T>
    void SetPath(T file){

        file_name_ = std::move(file);
    }

    void SerizalizeCatalog(const RequestHandler& rh );

    void DeserializeCatalog(transport::Catalogue& catalog,renderer::MapRenderer& renderer,router::TransportRouter& router);

private:

    renderer::RenderSettings DeserializeRendererSettings(const proto_catalogue_serialization::TransportCatalogue& catalog);

    svg::Point DeserializePoint(const proto_catalogue_serialization::Point& proto_point);
    svg::Color DeserializeColor(const proto_catalogue_serialization::Color& proto_color);
    //std::vector<graph::Edge<double> > edges, std::vector<std::vector<graph::EdgeId> > incidence_lists

    std::vector<graph::Edge<double> > DeserializeEdges(const proto_catalogue_serialization::Graf& proto_graf);
    std::vector<std::vector<graph::EdgeId> > DesrializeIncidenceLists(const proto_catalogue_serialization::Graf&  proto_graf);
    router::RoutingSettings DeserialezeRoutingSettings(const proto_catalogue_serialization::RouterSettings& proto_routing_settings);


    router::TransportRouter DeserializeRouter(const Catalogue& rh,const proto_catalogue_serialization::TransportRouter& proto_router);
    std::unordered_map<const Stop*, router::StopPairVertexId> DeserializeRouterStopToPairId(const Catalogue& cat,const proto_catalogue_serialization::StopToPairId& proto_stop_to_pair_id);
    std::unordered_map<graph::EdgeId, router::EdgeInfo> DeserializeRouterEdgeIdToType(const Catalogue& cat,const proto_catalogue_serialization::EdgeIdToType& proto_edge_id_to_type);
    router::EdgeInfo DeserializeEdgeInfo(const proto_catalogue_serialization::EdgeInfo& proto_edge_info);

    proto_catalogue_serialization::Point SerializedPoint(const svg::Point& point);
    proto_catalogue_serialization::Color SerializeColor(const svg::Color& color);
    proto_catalogue_serialization::RGB_color SerializeRGB(const svg::Rgb& rgba);
    proto_catalogue_serialization::RGBA_color SerializeRGBA(const svg::Rgba& rgb);


    proto_catalogue_serialization::RouterSettings SerializeRouterSettings(const router::RoutingSettings& settings );
    proto_catalogue_serialization::TransportRouter SerializeTransportRouter(const RequestHandler& rh);
    proto_catalogue_serialization::StopToPairId SerializeRouterStopToPairId(const RequestHandler& rh,const std::unordered_map<const Stop*, router::StopPairVertexId>& stop_to_pair_id)const;
    proto_catalogue_serialization::EdgeIdToType SerializeEdgeIdToType(const std::unordered_map<graph::EdgeId, router::EdgeInfo>& edge_id_to_type) const ;
    proto_catalogue_serialization::EdgeInfo SerializeEdgeInfo(const router::EdgeInfo& edge_info) const;

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

    std::filesystem::path file_name_;
};


}
}

