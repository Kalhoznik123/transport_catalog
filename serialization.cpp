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
std::ofstream out(file_name_,std::ios::binary);

catalog.SerializeToOstream(&out);

}

void Serializator::DeserializeCatalog(Catalogue &catalog,renderer::MapRenderer& renderer)
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


    if(proto_color.has_rgb()){
    auto    result = svg::Rgb{static_cast<uint8_t>(proto_color.rgb().red()),
                static_cast<uint8_t>(proto_color.rgb().green()),
                static_cast<uint8_t>(proto_color.rgb().blue())};
    return  result;
    }else if(proto_color.has_rgba()){
        auto result = svg::Rgba{static_cast<uint8_t>(proto_color.rgba().red()),
                static_cast<uint8_t>(proto_color.rgba().green()),
                static_cast<uint8_t>(proto_color.rgba().blue()),
                proto_color.rgba().opacity()};
    return  result;
    }else if(proto_color.has_string_color()){
        auto result = proto_color.string_color().color();
    return  result;
    }else{

    }

    throw std::runtime_error("Deserialized Color ERROR");
}

proto_catalogue_serialization::Point Serializator::SerializedPoint(const svg::Point& point_){
    proto_catalogue_serialization::Point point;
    point.set_x(point_.x);
    point.set_y(point_.y);

    return point;
}

proto_catalogue_serialization::Color Serializator::SerializedColor(const svg::Color &color){
    proto_catalogue_serialization::Color result;
    switch(color.index()){
    case 1:{
        proto_catalogue_serialization::String_color string_color;
        string_color.set_color(std::get<1>(color));
        *result.mutable_string_color() = std::move(string_color);
        break;
    }
    case 2:{
        *result.mutable_rgb() = SerializedRGB(std::get<2>(color));
        break;
    }
    case 3:{
        *result.mutable_rgba() = SerializedRGBA(std::get<3>(color));
        break;
    }

    }
    return  result;
}

proto_catalogue_serialization::RGB_color Serializator::SerializedRGB(const svg::Rgb &rgb)
{
    proto_catalogue_serialization::RGB_color result;
    result.set_red(rgb.red);
    result.set_blue(rgb.blue);
    result.set_green(rgb.green);

    return  result;

}

proto_catalogue_serialization::RGBA_color Serializator::SerializedRGBA(const svg::Rgba &rgba)
{
    proto_catalogue_serialization::RGBA_color result;
    result.set_red(rgba.red);
    result.set_blue(rgba.blue);
    result.set_green(rgba.green);
    result.set_opacity(rgba.opacity);
    return  result;

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
    *proto_settings.mutable_underlayer_color()  = SerializedColor(settings.underlayer_color);
    proto_settings.set_underlayer_width(settings.underlayer_width);

    for(const auto& color:settings.color_palette){
        *proto_settings.add_color_palette() = SerializedColor(color);

    }

    *catalog.mutable_rend_settings() = std::move(proto_settings);
}

void Serializator::DeserializeDistance(Catalogue &catalog, const proto_catalogue_serialization::TransportCatalogue &proto_catalog)
{
    int size = proto_catalog.distance_between_stops_size();

    for(int i =0 ;i<size;++i){
        const proto_catalogue_serialization::DistanceBetweenStops& proto_distance = proto_catalog.distance_between_stops(i);
        catalog.SetDistanceBeetweenStops(catalog.GetStopById(proto_distance.stop_from()).name,
                                         catalog.GetStopById(proto_distance.stop_to()).name,proto_distance.distance());

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
        bus.start_stop = &catalog.GetStopById(proto_bus.start_stop());
        bus.end_stop = &catalog.GetStopById(proto_bus.end_stop());

        const int stops_size = proto_bus.stops_size();

        for(int j = 0; j< stops_size; ++j){
            bus.stops.push_back(&catalog.GetStopById(proto_bus.stops(j)));
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
