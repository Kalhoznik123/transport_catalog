#include <stdexcept>
#include "serialization.h"
#include <unordered_map>
#include<fstream>


namespace transport {

namespace serialization {



void Serializator::SerizalizeCatalog(const RequestHandler& rh){

std::unordered_map<std::string, int> stop_name_to_index;
proto_catalogue_serialization::TransportCatalogue catalog;

SerializeStopsAndFillMap(catalog,rh,stop_name_to_index);
SerializeBuses(catalog,rh,stop_name_to_index);
SerializeDistancesBetweenStops(catalog,rh,stop_name_to_index);

SerializeRenderSettings(catalog,rh);

*catalog.mutable_transport_router() = SerializeTransportRouter(rh);

std::ofstream out(file_name_,std::ios::binary);
catalog.SerializeToOstream(&out);

}

void Serializator::DeserializeCatalog(Catalogue &catalog,renderer::MapRenderer& renderer, router::TransportRouter& router)
{
proto_catalogue_serialization::TransportCatalogue proto_catalog;
std::ifstream in(file_name_,std::ios::binary);

proto_catalog.ParseFromIstream(&in);

//if(!proto_catalog.ParseFromIstream(&in))
//    return;

DeserializeStops(catalog,proto_catalog);
DeserializeDistance(catalog,proto_catalog);
DeserializeBus(catalog,proto_catalog);
auto renderer_settings = DeserializeRendererSettings(proto_catalog);
renderer.setVisualisationSettings(std::move(renderer_settings));
router = DeserializeRouter(catalog,proto_catalog.transport_router());

}

renderer::RenderSettings Serializator::DeserializeRendererSettings(const proto_catalogue_serialization::TransportCatalogue& catalog)
{

    const proto_catalogue_serialization::RendererSettings&  proto_render_settings = catalog.rend_settings();
    renderer::RenderSettings settings;

    settings.width = proto_render_settings.width();
    settings.height = proto_render_settings.height();
    settings.padding = proto_render_settings.padding();
    settings.line_width = proto_render_settings.line_width();
    settings.stop_radius = proto_render_settings.stop_radius();
    settings.bus_label_offset = DeserializePoint(proto_render_settings.bus_label_offset());
    settings.underlayer_color = DeserializeColor(proto_render_settings.underlayer_color());
    settings.underlayer_width = proto_render_settings.underlayer_width();
    settings.stop_label_offset =  DeserializePoint(proto_render_settings.stop_label_offset());
    settings.bus_label_font_size = proto_render_settings.bus_label_font_size();
    settings.stop_label_font_size = proto_render_settings.stop_label_font_size();


for(int i = 0; i<proto_render_settings.color_palette_size();++i){
    settings.color_palette.push_back(DeserializeColor(proto_render_settings.color_palette(i)));
  }

return settings;
}

svg::Point Serializator::DeserializePoint(const proto_catalogue_serialization::Point &proto_point)
{
    return {proto_point.x(),proto_point.y()};
}

svg::Color Serializator::DeserializeColor(const proto_catalogue_serialization::Color& proto_color)
{


    if(proto_color.has_rgb_color()){
    auto    result = svg::Rgb{static_cast<uint8_t>(proto_color.rgb_color().red()),
                static_cast<uint8_t>(proto_color.rgb_color().green()),
                static_cast<uint8_t>(proto_color.rgb_color().blue())};
    return  result;
    }else if(proto_color.has_rgba_color()){
        auto result = svg::Rgba{static_cast<uint8_t>(proto_color.rgba_color().red()),
                static_cast<uint8_t>(proto_color.rgba_color().green()),
                static_cast<uint8_t>(proto_color.rgba_color().blue()),
                proto_color.rgba_color().opacity()};
    return  result;
    //}else if(proto_color.has_string_color()){
        //auto result = proto_color.string_color().color();
    //return  result;
    }else{
     auto result = proto_color.string_color();
    return  result;

    }

    throw std::runtime_error("Deserialized Color ERROR");
}

std::vector<graph::Edge<double> > Serializator::DeserializeEdges(const proto_catalogue_serialization::Graf&  proto_graf){

    std::vector<graph::Edge<double> > result;

    for(size_t i = 0; i< proto_graf.edges_size(); ++i){
        graph::Edge<double> edge;

        edge.from = proto_graf.edges(i).stop_from();
        edge.to =  proto_graf.edges(i).stop_to();
        edge.weight = proto_graf.edges(i).weight();

        //result.emplace_back(edges[i].stop_from(),edges[i].stop_to(),edges[i].weight());

        result.push_back(std::move(edge));
    }

    return  result;
}

std::vector<std::vector<graph::EdgeId> > Serializator::DesrializeIncidenceLists(const proto_catalogue_serialization::Graf&  proto_graf){

std::vector<std::vector<graph::EdgeId> > result;

for(size_t i = 0; i< proto_graf.incidence_lists_size(); ++i){
    std::vector<graph::EdgeId> incidence_list;
    for(size_t j = 0; j< proto_graf.incidence_lists(i).edgeid_size();++j){
        incidence_list.push_back(proto_graf.incidence_lists(i).edgeid(j));
    }
    result.push_back(std::move(incidence_list));
}

return result;

}

router::RoutingSettings Serializator::DeserialezeRoutingSettings(const proto_catalogue_serialization::RouterSettings &proto_routing_settings){

return {proto_routing_settings.bus_wait_time(),proto_routing_settings.bus_velocity()};

}

router::TransportRouter Serializator::DeserializeRouter(const transport::Catalogue& cat,const proto_catalogue_serialization::TransportRouter&proto_router){

    router::TransportRouter result;
    result.SetRoutingSettings(DeserialezeRoutingSettings(proto_router.router_settings()));

    auto edges = DeserializeEdges(proto_router.graph());
    auto incidence_lists = DesrializeIncidenceLists(proto_router.graph());
    auto stop_ptr_to_pair_id = DeserializeRouterStopToPairId(cat,proto_router.stop_to_pair_id());
    auto edge_id_to_type = DeserializeRouterEdgeIdToType(cat,proto_router.edge_id_to_type());

    result.BuildRouter(std::move(edges),
                       std::move(incidence_lists),
                       std::move(stop_ptr_to_pair_id),
                       std::move(edge_id_to_type));
    //result.BuildRouter(
    return result;
}

std::unordered_map<const Stop *, router::StopPairVertexId> Serializator::DeserializeRouterStopToPairId(const Catalogue& cat,
                                                                                                             const proto_catalogue_serialization::StopToPairId &proto_stop_to_pair_id){
    std::unordered_map<const Stop *, router::StopPairVertexId> result;
    for(int i = 0; i <proto_stop_to_pair_id.stop_id_size();++i){
        result[&cat.GetStopByIndex(proto_stop_to_pair_id.stop_id(i))] = {proto_stop_to_pair_id.stop_pair_vretex_id(i).bus_wait_begin(),proto_stop_to_pair_id.stop_pair_vretex_id(i).bus_wait_end()};
    }

    return result;
}

std::unordered_map<graph::EdgeId, router::EdgeInfo> Serializator::DeserializeRouterEdgeIdToType(const Catalogue& cat,
                                                                                                const proto_catalogue_serialization::EdgeIdToType &proto_edge_id_to_type){
    std::unordered_map<graph::EdgeId, router::EdgeInfo> result;
    for(int i = 0; i< proto_edge_id_to_type.edge_id_size();++i){
       result[proto_edge_id_to_type.edge_id(i)] = DeserializeEdgeInfo(proto_edge_id_to_type.edge_info(i));
    }
    return result;
}

router::EdgeInfo Serializator::DeserializeEdgeInfo(const proto_catalogue_serialization::EdgeInfo &proto_edge_info){
router::EdgeInfo result;

if(proto_edge_info.has_bus_edge_info()){
    router::BusEdgeInfo info;
    info.bus_name = proto_edge_info.bus_edge_info().bus_name();
    info.span_count = proto_edge_info.bus_edge_info().span_count();
    info.time = proto_edge_info.bus_edge_info().time();
    result = std::move(info);
    return result;
}else if(proto_edge_info.has_wait_edge_info()){
    router::WaitEdgeInfo info;
    info.stop_name = proto_edge_info.wait_edge_info().stop_name();
    info.time = proto_edge_info.wait_edge_info().time();
    result = std::move(info);
    return result;
}else{
throw std::runtime_error("DeserializeEdgeInfo ERROR");
}
}

proto_catalogue_serialization::Point Serializator::SerializedPoint(const svg::Point& point_){
    proto_catalogue_serialization::Point point;
    point.set_x(point_.x);
    point.set_y(point_.y);

    return point;
}

proto_catalogue_serialization::Color Serializator::SerializeColor(const svg::Color &color){
    proto_catalogue_serialization::Color result;
    switch(color.index()){
    case 1:{
        //proto_catalogue_serialization::String_color string_color;
        //string_color.set_color(std::get<1>(color));
        result.set_string_color(std::get<1>(color));
        //*result.mutable_string_color() = std::move(string_color);
        break;
    }
    case 2:{
        *result.mutable_rgb_color() = SerializeRGB(std::get<2>(color));
        //*result.mutable_rgb() = SerializeRGB(std::get<2>(color));
        break;
    }
    case 3:{
        *result.mutable_rgba_color() = SerializeRGBA(std::get<3>(color));
        //*result.mutable_rgba() = SerializeRGBA(std::get<3>(color));
        break;
    }

    }
    return  result;
}

proto_catalogue_serialization::RGB_color Serializator::SerializeRGB(const svg::Rgb &rgb)
{
    proto_catalogue_serialization::RGB_color result;
    result.set_red(rgb.red);
    result.set_blue(rgb.blue);
    result.set_green(rgb.green);

    return  result;

}

proto_catalogue_serialization::RGBA_color Serializator::SerializeRGBA(const svg::Rgba &rgba){

    proto_catalogue_serialization::RGBA_color result;
    result.set_red(rgba.red);
    result.set_blue(rgba.blue);
    result.set_green(rgba.green);
    result.set_opacity(rgba.opacity);
    return  result;

}

proto_catalogue_serialization::RouterSettings Serializator::SerializeRouterSettings(const router::RoutingSettings &settings){

    proto_catalogue_serialization::RouterSettings result;
    result.set_bus_velocity(settings.bus_velocity);
    result.set_bus_wait_time(settings.bus_wait_time);

    return result;
}

proto_catalogue_serialization::TransportRouter Serializator::SerializeTransportRouter(const RequestHandler &rh){

    proto_catalogue_serialization::TransportRouter proto_router;


    //add adges to proto_router
    for(const auto& edge: rh.GetRouterGrafEages()){
        proto_catalogue_serialization::Edge proto_edge;
        proto_edge.set_stop_from(edge.from);
        proto_edge.set_stop_to(edge.to);
        proto_edge.set_weight(edge.weight);

        *proto_router.mutable_graph()->add_edges() = std::move(proto_edge);
    }

    //add increase lists to proto_router
    for(const auto& increase_list : rh.GetRouterGrafIncidenceLists()){
        proto_catalogue_serialization::IncidenceList list;
        for(size_t edge_id: increase_list){
            list.add_edgeid(edge_id);
        }
        *proto_router.mutable_graph()->add_incidence_lists() = std::move(list);
    }

    *proto_router.mutable_router_settings() = SerializeRouterSettings(rh.GetRoutingSettings());
    *proto_router.mutable_stop_to_pair_id() = SerializeRouterStopToPairId(rh,rh.GetRouterStopToPairID());
    *proto_router.mutable_edge_id_to_type() = SerializeEdgeIdToType(rh.GetRouterEdgeIdToType());
    return proto_router;
}

proto_catalogue_serialization::StopToPairId Serializator::SerializeRouterStopToPairId(const RequestHandler& rh,
                                                                                      const std::unordered_map<const Stop *, router::StopPairVertexId> &stop_to_pair_id) const{
    proto_catalogue_serialization::StopToPairId result;

    for(const auto&[stop,stop_pair_vertex_id]:stop_to_pair_id){
        result.add_stop_id(rh.GetTransportCatalogStopIndex(*stop));

        proto_catalogue_serialization::StopPairVertexId proto_stop_pair_vertex_id;

        proto_stop_pair_vertex_id.set_bus_wait_begin(stop_pair_vertex_id.bus_wait_begin);
        proto_stop_pair_vertex_id.set_bus_wait_end(stop_pair_vertex_id.bus_wait_end);
        *result.add_stop_pair_vretex_id()  = std::move(proto_stop_pair_vertex_id);
    }

    return result;
}

proto_catalogue_serialization::EdgeIdToType Serializator::SerializeEdgeIdToType(const std::unordered_map<graph::EdgeId, router::EdgeInfo> &edge_id_to_type) const{
    //TODO: add this method, add deserialization metods
    proto_catalogue_serialization::EdgeIdToType result;
    for(const auto& [edge_id,edge_info]:edge_id_to_type){
        result.add_edge_id(edge_id);
        proto_catalogue_serialization::EdgeInfo proto_edge_info = SerializeEdgeInfo(edge_info);
        *result.add_edge_info() = std::move(proto_edge_info);
    }
    return result;
}

proto_catalogue_serialization::EdgeInfo Serializator::SerializeEdgeInfo(const router::EdgeInfo &edge_info) const{
    proto_catalogue_serialization::EdgeInfo result;
    switch (edge_info.index()) {
    case 0:{
        //busInfo
        proto_catalogue_serialization::WaitEdgeInfo info;
        info.set_stop_name(std::get<0>(edge_info).stop_name);
        info.set_time(std::get<0>(edge_info).time);
        *result.mutable_wait_edge_info() = std::move(info);
        break;
    }
    case 1:{
//waitInfo
        proto_catalogue_serialization::BusEdgeInfo info;
        info.set_bus_name( std::get<1>(edge_info).bus_name);
        info.set_time(std::get<1>(edge_info).time);
        info.set_span_count(std::get<1>(edge_info).span_count);
        *result.mutable_bus_edge_info() = std::move(info);
        break;
    }
    }
    return result;
}

void Serializator::SerializeBuses(proto_catalogue_serialization::TransportCatalogue &catalog,
                                  const RequestHandler& rh,
                                  const std::unordered_map<std::string, int> &stops_index){
    const auto& buses = rh.GetBuses();

    for(const auto& bus :buses){
        proto_catalogue_serialization::Bus proto_bus;
        proto_bus.set_name(bus.name);
        proto_bus.set_start_stop(stops_index.at(bus.start_stop->name));
        proto_bus.set_end_stop(stops_index.at(bus.end_stop->name));
        proto_bus.set_is_roundtrip(bus.is_roundtrip);

        for(const Stop* stop: bus.stops){
            proto_bus.add_stops(stops_index.at(stop->name));
        }

        *catalog.add_buses() = std::move(proto_bus);
    }
}

void Serializator::SerializeDistancesBetweenStops(proto_catalogue_serialization::TransportCatalogue &catalog,
                                                  const RequestHandler &rh,
                                                  const std::unordered_map<std::string, int> &stops_index){
    const auto& distances = rh.GetDistancesBetweenStops();

    for(const auto& [stops_pair, distance]: distances){
        proto_catalogue_serialization::DistanceBetweenStops proto_distance;

        proto_distance.set_stop_from(stops_index.at(stops_pair.first->name));
        proto_distance.set_stop_to(stops_index.at(stops_pair.second->name));
        proto_distance.set_distance(distance);
        *catalog.add_distance_between_stops() = std::move(proto_distance);
    }

}

void Serializator::SerializeRenderSettings(proto_catalogue_serialization::TransportCatalogue &catalog, const RequestHandler &rh){
    proto_catalogue_serialization::RendererSettings proto_settings;
    const auto& settings = rh.GetRendererSettings();

    proto_settings.set_width(settings.width);
    proto_settings.set_height(settings.height);
    proto_settings.set_padding(settings.padding);
    proto_settings.set_line_width(settings.line_width);
    proto_settings.set_stop_radius(settings.stop_radius);
    proto_settings.set_stop_label_font_size(settings.stop_label_font_size);
    proto_settings.set_bus_label_font_size(settings.bus_label_font_size);

    *proto_settings.mutable_bus_label_offset() = SerializedPoint(settings.bus_label_offset);

    *proto_settings.mutable_stop_label_offset() = SerializedPoint(settings.stop_label_offset);
    *proto_settings.mutable_underlayer_color()  = SerializeColor(settings.underlayer_color);
    proto_settings.set_underlayer_width(settings.underlayer_width);

    for(const auto& color:settings.color_palette){
        *proto_settings.add_color_palette() = SerializeColor(color);

    }

    *catalog.mutable_rend_settings() = std::move(proto_settings);
}

void Serializator::DeserializeDistance(Catalogue &catalog, const proto_catalogue_serialization::TransportCatalogue &proto_catalog)
{
    int size = proto_catalog.distance_between_stops_size();

    for(int i =0 ;i<size;++i){
        const proto_catalogue_serialization::DistanceBetweenStops& proto_distance = proto_catalog.distance_between_stops(i);
        catalog.SetDistanceBeetweenStops(catalog.GetStopByIndex(proto_distance.stop_from()).name,
                                         catalog.GetStopByIndex(proto_distance.stop_to()).name,proto_distance.distance());

    }
}

void Serializator::DeserializeBus(Catalogue &catalog,
                                  const proto_catalogue_serialization::TransportCatalogue &proto_catalog)
{
    const int size = proto_catalog.buses_size();
    for(int i = 0; i< size;++i){
        const proto_catalogue_serialization::Bus& proto_bus = proto_catalog.buses(i);
        Bus bus;
        bus.name = proto_bus.name();
        bus.is_roundtrip = proto_bus.is_roundtrip();
        bus.start_stop = &catalog.GetStopByIndex(proto_bus.start_stop());
        bus.end_stop = &catalog.GetStopByIndex(proto_bus.end_stop());

        const int stops_size = proto_bus.stops_size();

        for(int j = 0; j< stops_size; ++j){
            bus.stops.push_back(&catalog.GetStopByIndex(proto_bus.stops(j)));
        }
        catalog.AddBus(std::move(bus));
    }
}

void Serializator::DeserializeStops(Catalogue& catalog, const proto_catalogue_serialization::TransportCatalogue &proto_catalog)
{
    int size = proto_catalog.stops_size();

 for(int i = 0 ; i<size; ++i){
     const proto_catalogue_serialization::Stop& proto_stop = proto_catalog.stops(i);
     Stop stop;
     stop.name = proto_stop.name();
     stop.coordinate.lat = proto_stop.coordinates().lat();
     stop.coordinate.lng = proto_stop.coordinates().lng();

     catalog.AddStop(stop);
 }


}

void Serializator::SerializeStopsAndFillMap(proto_catalogue_serialization::TransportCatalogue &catalog,
                                            const RequestHandler &rh,
                                            std::unordered_map<std::string, int>& stops_index)
{
    const auto& stops = rh.GetStops();
    int index = 0 ;
    for(const auto& stop:stops){
        stops_index[stop.name] = index++;
        proto_catalogue_serialization::Stop proto_stop;
        proto_stop.set_name(stop.name);
        proto_stop.mutable_coordinates()->set_lat(stop.coordinate.lat);
        proto_stop.mutable_coordinates()->set_lng(stop.coordinate.lng);
        *catalog.add_stops() = std::move(proto_stop);
    }
}



}
}
