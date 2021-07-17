#include "json_reader.h"
#include <stdexcept>
#include <iostream>
#include <set>

using namespace std::string_literals;

namespace json {

Reader::Reader( transport::Catalogue& catalogue, renderer::MapRenderer& renderer)
    :catalogue_(catalogue)
    ,renderer_(renderer) {

}

void Reader::ParseJson(std::istream& input) {
  Document json = Load(input);
  std::map<std::string, Node> nodes = std::move(json.GetRoot().AsMap());
  base_requests_ = std::move(nodes.at("base_requests"s).AsArray());

  render_settings_ = std::move(nodes.at("render_settings"s).AsMap());

  stat_requests_ = std::move(nodes.at("stat_requests"s).AsArray());

  AddStops();
  AddDistances();
  AddBuses();

  renderer_.setVisualisationSettings(parseVisualisationSettings());
}

void Reader::PrintJson(std::ostream& out) const {

  Print(returnStat(), out);
}

void Reader::AddStops() const {
  for (const Node& node : base_requests_) {
    const auto& node_map = node.AsMap();
    if (node_map.at("type"s).AsString() == "Stop") {
      catalogue_.AddStop({node_map.at("name"s).AsString(), {node_map.at("latitude"s).AsDouble(), node_map.at("longitude"s).AsDouble()}});
    }
  }
}

void Reader::AddDistances() const {
  for (const Node& node : base_requests_) {

    const auto& node_map = node.AsMap();

    if (node_map.at("type"s).AsString() == "Stop") {

      auto map_distances = node_map.at("road_distances"s).AsMap();
      std::string from = std::move(node_map.at("name"s).AsString());
      for (const auto& [to,distance] : map_distances) {
        catalogue_.SetDistanceBeetweenStops(std::move(from),std::move(to),std::move(distance.AsDouble()));
      }
    }
  }
}

void Reader::AddBuses() const {
  for (const Node& node : base_requests_) {

    auto node_map = std::move(node.AsMap());


    transport::Bus bus;
    if (node_map.at("type"s).AsString() == "Bus") {

      bus.name =node_map.at("name"s).AsString();


      for (const Node& stop : node_map.at("stops"s).AsArray()) {

        bus.stops.push_back(catalogue_.GetStop(stop.AsString()));
      }

      bus.start_stop =  bus.stops.size()== 0 ? nullptr: bus.stops[0];
      bus.end_stop =  bus.stops.size()== 0 ? nullptr: bus.stops[bus.stops.size() - 1];

      if (!node_map.at("is_roundtrip"s).AsBool()&&bus.start_stop!=0) {
        int size = static_cast<int>(bus.stops.size());
        for (int i = size - 2; i >= 0; --i ) {
          bus.stops.push_back(bus.stops[i]);
        }
      }
      catalogue_.AddBus(std::move(bus));
    }
  }
}

Node Reader::stopStat(const Node& node) const {
  Dict answer;
  Array buses;
  std::set<std::string> buses_set;

  RequestHandler handler(catalogue_,renderer_);

  std::optional<transport::stopInfoStruct> buses_ptr = handler.GetBusesByStop(std::move(node.AsMap().at("name"s).AsString()));
  if (!buses_ptr) {
    answer["request_id"s] = Node(std::move(node.AsMap().at("id"s).AsInt()));
    answer["error_message"s] = Node("not found"s);
    return Node(answer);
  }
  for (const transport::Bus* bus_ptr : buses_ptr->buses) {
    if (bus_ptr) {
      buses_set.insert(bus_ptr->name);
    }
  }
  for (const std::string& bus_mane : buses_set) {
    buses.push_back(Node(bus_mane));
  }
  answer["buses"s] = Node(buses);
  answer["request_id"s] = Node(std::move(node.AsMap().at("id"s).AsInt()));
  return Node(answer);
}

Node Reader::routeStat(const Node& node) const {
  Dict answer;
  RequestHandler handler(catalogue_,renderer_);
  std::optional<transport::BusInformation> route_ptr = handler.GetBusStat(std::move(node.AsMap().at("name"s).AsString()));
  if (!route_ptr) {
    answer["request_id"s] = Node(std::move(node.AsMap().at("id"s).AsInt()));
    answer["error_message"s] = Node("not found"s);
    return Node(answer);
  }
  answer["curvature"s] = Node(route_ptr->curvature);
  answer["route_length"s] = Node(route_ptr->route_length);
  answer["stop_count"s] = Node(route_ptr->stops);
  answer["unique_stop_count"s] = Node(route_ptr->unique_stops);
  answer["request_id"s] = Node(std::move(node.AsMap().at("id"s).AsInt()));
  return Node(answer);
}

Node Reader::mapStat(const Node& node) const {
  Dict map;
  svg::Document renderes_map;
  RequestHandler handler(catalogue_,renderer_);
  handler.RenderMap(renderes_map);
  std::stringstream ss;
  {
    renderes_map.Render(ss);
  }
  map["map"s] = Node(ss.str());
  map["request_id"s] = Node(std::move(node.AsMap().at("id"s).AsInt()));
  return Node(map);
}

Document Reader::returnStat() const {
  // обрабатываем каждый запрос
  Array array;
  for (const Node& node : stat_requests_) {
    const std::string& type = node.AsMap().at("type"s).AsString();
    if (type == "Map"s) {
      array.push_back(mapStat(node));
    }
    if (type == "Stop"s) {
      array.push_back(stopStat(node));
    }
    if (type == "Bus"s) {
      array.push_back(routeStat(node));
    }
  }
  return Document(Node(array));
}


renderer::RenderSettings Reader::parseVisualisationSettings() const {
  renderer::RenderSettings return_settings;
  return_settings.width = std::move(render_settings_.at("width"s).AsDouble());
  return_settings.height = std::move(render_settings_.at("height"s).AsDouble());
  return_settings.padding = std::move(render_settings_.at("padding"s).AsDouble());
  return_settings.line_width = std::move(render_settings_.at("line_width"s).AsDouble());
  return_settings.stop_radius = std::move(render_settings_.at("stop_radius"s).AsDouble());
  return_settings.bus_label_font_size = std::move(render_settings_.at("bus_label_font_size"s).AsInt());
  return_settings.bus_label_offset = {std::move(render_settings_.at("bus_label_offset"s).AsArray().at(0).AsDouble()),std::move(render_settings_.at("bus_label_offset"s).AsArray().at(1).AsDouble())};
  return_settings.stop_label_font_size = std::move(render_settings_.at("stop_label_font_size"s).AsInt());
  return_settings.stop_label_offset ={std::move(render_settings_.at("stop_label_offset"s).AsArray().at(0).AsDouble()),std::move(render_settings_.at("stop_label_offset"s).AsArray().at(1).AsDouble())};
  return_settings.underlayer_color = ParseColor(render_settings_.at("underlayer_color"s));
  return_settings.underlayer_width = std::move(render_settings_.at("underlayer_width"s).AsDouble());
  Array color_palette = std::move(render_settings_.at("color_palette"s).AsArray());
  return_settings.color_palette.clear();
  for (const Node& node : color_palette) {
    return_settings.color_palette.push_back(ParseColor(node));
  }
  return return_settings;
}

// возвращает цвет из узла
svg::Color Reader::ParseColor(Node node) const {
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

} // namespace json
