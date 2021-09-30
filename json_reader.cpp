#include "json_reader.h"
#include "json_builder.h"
#include "request_handler.h"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <set>

using namespace std::string_literals;
using namespace json;
namespace transport {

namespace detail {


struct EdgeInfoGetter {
    json::Node operator()(const router::WaitEdgeInfo& edge_info);
    json::Node operator()(const router::BusEdgeInfo& edge_info);
};

}


Reader::Reader( transport::Catalogue& catalogue, renderer::MapRenderer& renderer,
                router::TransportRouter& router,serialization::Serializator& serializator)
    : catalogue_(catalogue)
    , renderer_(renderer)
    , router_(router)
    , serializator_(serializator)
{

}

void Reader::ParseRequests(std::istream& input) {
    Document json = json::Load(input);
    std::map<std::string, Node> nodes = std::move(json.GetRoot().AsDict());

    if(auto it = nodes.find("base_requests"s);it !=nodes.end() ){

        base_requests_ = std::move(it->second.AsArray());

        AddStops();
        AddDistances();
        AddBuses();

    }
    if(auto it = nodes.find("render_settings"s);it !=nodes.end() ){
        render_settings_ = std::move(it->second.AsDict());
        renderer_.setVisualisationSettings(ParseVisualisationSettings());

    }
    if(auto it = nodes.find("stat_requests"s);it !=nodes.end() ){
        stat_requests_ = std::move(it->second.AsArray());

        //stat_requests_ = std::move(nodes.at("stat_requests"s).AsArray());

    }

    if(auto it = nodes.find("routing_settings"s);it !=nodes.end() ){
        route_settings_ = std::move(it->second.AsDict());
        //route_settings_ = std::move(nodes.at("routing_settings").AsDict());
        router_.SetRoutingSettings(ParseRoutingSettings());
    }

    if(auto it = nodes.find("serialization_settings"s);it !=nodes.end() ){
        serialization_settings = std::move(it->second.AsDict());
        serializator_.SetPath(GetSerializationSettings());


    }

}

void Reader::PrintReply(std::ostream& out) const {

    Print(ReturnStat(), out);
}

std::string Reader::GetSerializationSettings() const
{

    auto result = serialization_settings.at("file").AsString();

    return result;
}


void Reader::AddStops() const {
  for (const Node& node : base_requests_) {
    const auto& node_map = node.AsDict();
    if (node_map.at("type"s).AsString() == "Stop") {
      catalogue_.AddStop({node_map.at("name"s).AsString(), {node_map.at("latitude"s).AsDouble(), node_map.at("longitude"s).AsDouble()}});
    }
  }
}

void Reader::AddDistances() const {
  for (const Node& node : base_requests_) {

    const auto& node_map = node.AsDict();

    if (node_map.at("type"s).AsString() == "Stop") {

      auto& map_distances = node_map.at("road_distances"s).AsDict();

      std::string from = std::move(node_map.at("name"s).AsString());
      for (auto& [to,distance] : map_distances) {
        catalogue_.SetDistanceBeetweenStops(from,std::move(to),std::move(distance.AsDouble()));
      }
    }
  }
}

void Reader::AddBuses() const {
  for (const Node& node : base_requests_) {

    const auto& node_map = node.AsDict();

    if (node_map.at("type"s).AsString() == "Bus") {
      transport::Bus bus;

      bus.name =node_map.at("name"s).AsString();

      for (const Node& stop : node_map.at("stops"s).AsArray()) {

        bus.stops.push_back(catalogue_.GetStop(stop.AsString()));
      }

      bus.start_stop =  bus.stops.size()== 0 ? nullptr: bus.stops[0];
      bus.end_stop =  bus.stops.size()== 0 ? nullptr: bus.stops[bus.stops.size() - 1];
      bus.is_roundtrip = node_map.at("is_roundtrip"s).AsBool();
      if (!node_map.at("is_roundtrip"s).AsBool()&&bus.start_stop!=nullptr) {
        const  int size = static_cast<int>(bus.stops.size());
        for (int i = size - 2; i >= 0; --i ) {
          bus.stops.push_back(bus.stops[i]);
        }
      }
      catalogue_.AddBus(std::move(bus));
    }
  }
}

json::Node Reader::StopStat(const Node& node) const {
  Array buses;
  std::set<std::string> buses_set;
  Node answer;
  RequestHandler handler(catalogue_,renderer_,router_);

  const std::optional<transport::StopInformation> buses_ptr = handler.GetBusesByStop(node.AsDict().at("name"s).AsString());
  if (!buses_ptr) {

   answer = Builder{}.StartDict().Key("request_id"s).Value(node.AsDict().at("id"s).AsInt())
                     .Key("error_message"s).Value("not found"s).EndDict().Build();

   return answer;
  }

  for (const transport::Bus* bus_ptr : buses_ptr->buses) {
    if (bus_ptr) {
      buses_set.insert(bus_ptr->name);
    }
  }

  for (const std::string& bus_mane : buses_set) {
    buses.push_back(json::Node(bus_mane));
  }

  answer =  Builder{}.StartDict().Key("buses"s).Value(json::Array(buses)).Key("request_id"s).Value(node.AsDict().at("id"s).AsInt()).EndDict().Build();

  return answer;

}

json::Node Reader::BusStat(const json::Node& node) const {
  json::Node answer;
  RequestHandler handler(catalogue_,renderer_,router_);
  const std::optional<transport::BusInformation> route_ptr = handler.GetBusStat(node.AsDict().at("name"s).AsString());
  if (!route_ptr) {
    answer = Builder{}.StartDict().Key("request_id"s).Value(node.AsDict().at("id"s).AsInt())
                     .Key("error_message"s).Value("not found"s).EndDict().Build();

    return answer;

  }

  answer =Builder{}.StartDict()
                   .Key("curvature"s).Value(route_ptr->curvature)
                   .Key("route_length"s).Value(route_ptr->route_length)
                   .Key("stop_count"s).Value(route_ptr->stops)
                   .Key("unique_stop_count"s).Value(route_ptr->unique_stops)
                   .Key("request_id"s).Value(node.AsDict().at("id"s).AsInt())
                   .EndDict()
               .Build();
  return answer;

}

json::Node Reader::MapStat(const json::Node& node) const {

  json::Node new_answer;
  svg::Document renderes_map;
  RequestHandler handler(catalogue_,renderer_,router_);
  handler.RenderMap(renderes_map);
  std::stringstream ss;

  renderes_map.Render(ss);

  return json::Builder{}.StartDict()
                        .Key("map"s).Value(ss.str())
                        .Key("request_id"s).Value(node.AsDict().at("id"s).AsInt())
      .EndDict().Build();
}

Node Reader::RouteStat(const Node &node) const
{


  const auto request = node.AsDict();

  RequestHandler handler(catalogue_,renderer_,router_);

  const std::optional<router::RouteInfo> route_info = handler.GetRouteInfo(request.at("from").AsString(),request.at("to").AsString());
  if(!route_info){
    return Builder{}.StartDict()
        .Key("request_id"s).Value(request.at("id"s).AsInt())
        .Key("error_message"s).Value("not found"s)
        .EndDict()
        .Build();
  }

  Array items;
  for (const auto& item : route_info->edges) {
    items.push_back(std::visit(detail::EdgeInfoGetter{}, item));
  }

  return Builder{}.StartDict()
      .Key("request_id"s).Value(request.at("id"s).AsInt())
      .Key("total_time"s).Value(route_info->total_time)
      .Key("items"s).Value(std::move(items))
      .EndDict()
      .Build();


}

json::Document Reader::ReturnStat() const {
  // обрабатываем каждый запрос
  using namespace std::string_view_literals;
  Array array;
  Node result;

  auto builder = Builder{};
  auto json = builder.StartArray();
  for (const Node& node : stat_requests_) {
    const std::string& type = node.AsDict().at("type"s).AsString();
    if (type == "Map"sv) {
      json.Value(MapStat(node).AsDict());

    }
    if (type == "Stop"sv) {
      json.Value(StopStat(node).AsDict());

    }
    if (type == "Bus"sv) {
      json.Value(BusStat(node).AsDict());

    }
    if(type == "Route"sv){
      json.Value(RouteStat(node).AsDict());
    }

  }
  return json::Document(json.EndArray().Build());
}

router::RoutingSettings Reader::ParseRoutingSettings(){
  using namespace router;

  RoutingSettings result;
  result.bus_wait_time = route_settings_.at("bus_wait_time").AsInt();
  result.bus_velocity = route_settings_.at("bus_velocity").AsDouble();

  return result;
}


renderer::RenderSettings Reader::ParseVisualisationSettings() const {
  renderer::RenderSettings return_settings;
  return_settings.width = std::move(render_settings_.at("width"s).AsDouble());
  return_settings.height = std::move(render_settings_.at("height"s).AsDouble());
  return_settings.padding = std::move(render_settings_.at("padding"s).AsDouble());
  return_settings.line_width = std::move(render_settings_.at("line_width"s).AsDouble());
  return_settings.stop_radius = std::move(render_settings_.at("stop_radius"s).AsDouble());
  return_settings.bus_label_font_size = std::move(render_settings_.at("bus_label_font_size"s).AsInt());
  {
  Array arr = render_settings_.at("bus_label_offset"s).AsArray();
  return_settings.bus_label_offset = {std::move(arr.at(0).AsDouble()),std::move(arr.at(1).AsDouble())};
  }
  return_settings.stop_label_font_size = std::move(render_settings_.at("stop_label_font_size"s).AsInt());
  {
    Array arr = render_settings_.at("stop_label_offset"s).AsArray();
    return_settings.stop_label_offset ={std::move(arr.at(0).AsDouble()),std::move(arr.at(1).AsDouble())};
  }
  return_settings.underlayer_color = ParseColor(render_settings_.at("underlayer_color"s));
  return_settings.underlayer_width = std::move(render_settings_.at("underlayer_width"s).AsDouble());
  const Array color_palette = std::move(render_settings_.at("color_palette"s).AsArray());
  return_settings.color_palette.clear();
  for (const Node& node : color_palette) {
    return_settings.color_palette.push_back(ParseColor(node));
  }
  return return_settings;
}

// возвращает цвет из узла
 svg::Color Reader::ParseColor(json::Node node)  {
   if (node.IsString()) {
     return std::move(node.AsString());
   }
   else if (node.IsArray()) {
     Array color_items = std::move(node.AsArray());
     if (color_items.size() == 4) {
       return svg::Rgba(std::move(color_items[0].AsInt()),
                        std::move(color_items[1].AsInt()),
                        std::move(color_items[2].AsInt()),
                        std::move(color_items[3].AsDouble()));
     }
     else {
       return svg::Rgb(std::move(color_items[0].AsInt()),
                       std::move(color_items[1].AsInt()),
                      std::move(color_items[2].AsInt()));
    }
  }
  else {
    return svg::NoneColor;
  }
 }
 namespace detail {


 Node EdgeInfoGetter::operator()(const router::WaitEdgeInfo &edge_info) {
   using namespace std::literals;
   return json::Builder{}.StartDict()
       .Key("type"s).Value("Wait"s)
       .Key("stop_name"s).Value(std::string(edge_info.stop_name))
       .Key("time").Value(edge_info.time)
       .EndDict()
       .Build();
 }

 Node EdgeInfoGetter::operator()(const router::BusEdgeInfo &edge_info) {
   using namespace std::literals;
   return json::Builder{}.StartDict()
       .Key("type"s).Value("Bus"s)
       .Key("bus"s).Value(std::string(edge_info.bus_name))
       .Key("span_count"s).Value(static_cast<int>(edge_info.span_count))
       .Key("time").Value(edge_info.time)
       .EndDict()
       .Build();
 }
 }

 } // namespace json
