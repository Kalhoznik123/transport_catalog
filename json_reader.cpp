#include "json_reader.h"
#include "json_builder.h"
#include <stdexcept>
#include <iostream>
#include <set>

using namespace std::string_literals;

namespace transport {

Reader::Reader( transport::Catalogue& catalogue, renderer::MapRenderer& renderer)
    :catalogue_(catalogue)
    ,renderer_(renderer) {

}

void Reader::FillCatalog(std::istream& input) {
  json::Document json = json::Load(input);
  std::map<std::string, json::Node> nodes = std::move(json.GetRoot().AsDict());
  base_requests_ = std::move(nodes.at("base_requests"s).AsArray());

  render_settings_ = std::move(nodes.at("render_settings"s).AsDict());

  stat_requests_ = std::move(nodes.at("stat_requests"s).AsArray());

  AddStops();
  AddDistances();
  AddBuses();

  renderer_.setVisualisationSettings(ParseVisualisationSettings());
}

void Reader::PrintRequests(std::ostream& out) const {

  Print(returnStat(), out);
}

void Reader::AddStops() const {
  for (const json::Node& node : base_requests_) {
    const auto& node_map = node.AsDict();
    if (node_map.at("type"s).AsString() == "Stop") {
      catalogue_.AddStop({node_map.at("name"s).AsString(), {node_map.at("latitude"s).AsDouble(), node_map.at("longitude"s).AsDouble()}});
    }
  }
}

void Reader::AddDistances() const {
  for (const json::Node& node : base_requests_) {

    const auto& node_map = node.AsDict();

    if (node_map.at("type"s).AsString() == "Stop") {

      auto map_distances = node_map.at("road_distances"s).AsDict();
      std::string from = std::move(node_map.at("name"s).AsString());
      for (auto& [to,distance] : map_distances) {
        catalogue_.SetDistanceBeetweenStops(std::move(from),std::move(to),std::move(distance.AsDouble()));
      }
    }
  }
}

void Reader::AddBuses() const {
  for (const json::Node& node : base_requests_) {

   const  auto node_map = std::move(node.AsDict());


    if (node_map.at("type"s).AsString() == "Bus") {
      transport::Bus bus;

      bus.name =node_map.at("name"s).AsString();


      for (const json::Node& stop : node_map.at("stops"s).AsArray()) {

        bus.stops.push_back(catalogue_.GetStop(stop.AsString()));
      }

      bus.start_stop =  bus.stops.size()== 0 ? nullptr: bus.stops[0];
      bus.end_stop =  bus.stops.size()== 0 ? nullptr: bus.stops[bus.stops.size() - 1];

      if (!node_map.at("is_roundtrip"s).AsBool()&&bus.start_stop!=0) {
       const  int size = static_cast<int>(bus.stops.size());
        for (int i = size - 2; i >= 0; --i ) {
          bus.stops.push_back(bus.stops[i]);
        }
      }
      catalogue_.AddBus(std::move(bus));
    }
  }
}

json::Node Reader::StopStat(const json::Node& node) const {
  json::Array buses;
  std::set<std::string> buses_set;
  json::Node answer;
  RequestHandler handler(catalogue_,renderer_);

  const std::optional<transport::StopInformation> buses_ptr = handler.GetBusesByStop(node.AsDict().at("name"s).AsString());
  if (!buses_ptr) {

   answer = json::Builder{}.StartDict().Key("request_id"s).Value(node.AsDict().at("id"s).AsInt())
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

  answer =  json::Builder{}.StartDict().Key("buses"s).Value(json::Array(buses)).Key("request_id"s).Value(node.AsDict().at("id"s).AsInt()).EndDict().Build();

  return answer;

}

json::Node Reader::RouteStat(const json::Node& node) const {
  json::Node answer;
  RequestHandler handler(catalogue_,renderer_);
  const std::optional<transport::BusInformation> route_ptr = handler.GetBusStat(node.AsDict().at("name"s).AsString());
  if (!route_ptr) {
    answer = json::Builder{}.StartDict().Key("request_id"s).Value(node.AsDict().at("id"s).AsInt())
                     .Key("error_message"s).Value("not found"s).EndDict().Build();

    return answer;

  }

  answer =  json::Builder{}.StartDict()
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
  RequestHandler handler(catalogue_,renderer_);
  handler.RenderMap(renderes_map);
  std::stringstream ss;

    renderes_map.Render(ss);

    return json::Builder{}.StartDict()
      .Key("map"s).Value(ss.str())
      .Key("request_id"s).Value(node.AsDict().at("id"s).AsInt()).EndDict().Build();
 }

json::Document Reader::returnStat() const {
  // обрабатываем каждый запрос
  json::Array array;
  json::Node result;

  auto builder = json::Builder{};
  auto json = builder.StartArray();
  for (const json::Node& node : stat_requests_) {
    const std::string& type = node.AsDict().at("type"s).AsString();
    if (type == "Map"s) {
      json.Value(MapStat(node).AsDict());

    }
    if (type == "Stop"s) {
      json.Value(StopStat(node).AsDict());

    }
    if (type == "Bus"s) {
      json.Value(RouteStat(node).AsDict());

    }
  }
  return json::Document(json.EndArray().Build());
}


renderer::RenderSettings Reader::ParseVisualisationSettings() const {
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
  json::Array color_palette = std::move(render_settings_.at("color_palette"s).AsArray());
  return_settings.color_palette.clear();
  for (const json::Node& node : color_palette) {
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
    json::Array color_items = std::move(node.AsArray());
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
