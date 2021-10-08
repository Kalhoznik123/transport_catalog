#pragma once
#include <filesystem>
#include <variant>
#include <string>
#include <transport_catalogue.pb.h>
#include <unordered_map>
#include "map_renderer.h"
#include "request_handler.h"

namespace transport {
class Catalogue;
namespace serialization {



class Serializator{
public:
    template<class T>
    void SetPath(T file){

        file_ = std::move(file);
    }

    void Serizalize(const Catalogue& ctl,const renderer::MapRenderer& renderer,const router::TransportRouter& router);

    void Deserialize(transport::Catalogue& catalog,renderer::MapRenderer& renderer,router::TransportRouter& router);

private:

    static  renderer::RenderSettings DeserializeRendererSettings(const proto_catalogue_serialization::TransportCatalogue& catalog);

    static  svg::Point DeserializePoint(const proto_catalogue_serialization::Point& proto_point);
    static  svg::Color DeserializeColor(const proto_catalogue_serialization::Color& proto_color);


    static   std::vector<graph::Edge<double> > DeserializeEdges(const proto_catalogue_serialization::Graf& proto_graf);
    static   std::vector<std::vector<graph::EdgeId> > DesrializeIncidenceLists(const proto_catalogue_serialization::Graf&  proto_graf);
    static    router::RoutingSettings DeserialezeRoutingSettings(const proto_catalogue_serialization::RouterSettings& proto_routing_settings);


    static   router::TransportRouter DeserializeRouter(const Catalogue& rh,const proto_catalogue_serialization::TransportRouter& proto_router);
    static   std::unordered_map<const Stop*, router::StopPairVertexId> DeserializeRouterStopToPairId(const Catalogue& cat,const proto_catalogue_serialization::StopToPairId& proto_stop_to_pair_id);
    static   std::unordered_map<graph::EdgeId, router::EdgeInfo> DeserializeRouterEdgeIdToType(const Catalogue& cat,const proto_catalogue_serialization::EdgeIdToType& proto_edge_id_to_type);
    static    router::EdgeInfo DeserializeEdgeInfo(const proto_catalogue_serialization::EdgeInfo& proto_edge_info);

    static    proto_catalogue_serialization::Point SerializedPoint(const svg::Point& point);
    static    proto_catalogue_serialization::Color SerializeColor(const svg::Color& color);
    static   proto_catalogue_serialization::RGB_color SerializeRGB(const svg::Rgb& rgba);
    static    proto_catalogue_serialization::RGBA_color SerializeRGBA(const svg::Rgba& rgb);


    proto_catalogue_serialization::RouterSettings SerializeRouterSettings(const router::RoutingSettings& settings );
    proto_catalogue_serialization::TransportRouter SerializeTransportRouter(const router::TransportRouter &rh,const transport::Catalogue& ctl);
    proto_catalogue_serialization::StopToPairId SerializeRouterStopToPairId(const transport::Catalogue& rh,const std::unordered_map<const Stop*, router::StopPairVertexId>& stop_to_pair_id)const;
    proto_catalogue_serialization::EdgeIdToType SerializeEdgeIdToType(const std::unordered_map<graph::EdgeId, router::EdgeInfo>& edge_id_to_type) const ;
    proto_catalogue_serialization::EdgeInfo SerializeEdgeInfo(const router::EdgeInfo& edge_info) const;

    static void SerializeStopsAndFillMap(proto_catalogue_serialization::TransportCatalogue& catalog,
                                         const transport::Catalogue& rh,
                                         std::unordered_map<std::string,int>& stops_index);
    static void SerializeBuses(proto_catalogue_serialization::TransportCatalogue& catalog,
                               const Catalogue& ctl,
                               const std::unordered_map<std::string,int>& stops_index);

    static void SerializeDistancesBetweenStops(proto_catalogue_serialization::TransportCatalogue& catalog,
                                               const Catalogue &ctl,
                                               const std::unordered_map<std::string,int>& stops_index);

    static void SerializeRenderSettings(proto_catalogue_serialization::TransportCatalogue& catalog,const renderer::MapRenderer &renderer);

    static void DeserializeStops(transport::Catalogue& catalog,const proto_catalogue_serialization::TransportCatalogue& proto_catalog);

    static void DeserializeDistances(transport::Catalogue& catalog,const proto_catalogue_serialization::TransportCatalogue& proto_catalog);

    static void DeserializeBuses(transport::Catalogue& catalog,const proto_catalogue_serialization::TransportCatalogue& proto_catalog);

    std::filesystem::path file_;
};


}
}

