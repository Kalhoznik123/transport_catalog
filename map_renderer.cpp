#include "map_renderer.h"

using namespace std::string_literals;
namespace transport {


namespace renderer {

void MapRenderer::setVisualisationSettings(RenderSettings settings) {
    settings_ = std::move(settings);
}

const RenderSettings &MapRenderer::GetRenderSetting() const{
    return settings_;
}

svg::Polyline MapRenderer::MakeBusRouteLine(size_t color_count) const {
  svg::Polyline bus_line;
  bus_line.SetFillColor(svg::NoneColor)
      .SetStrokeWidth(settings_.line_width)
      .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
      .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
      .SetStrokeColor(settings_.color_palette.at(color_count));
  return  bus_line;
}

svg::Text MapRenderer::MakeBusRouteNumberBackground(const std::string &name, const svg::Point &position) const{
  svg::Text stop_background;
  stop_background.SetData(name)
      .SetFillColor(settings_.underlayer_color)
      .SetStrokeColor(settings_.underlayer_color)
      .SetStrokeWidth(settings_.underlayer_width)
      .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
      .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
      .SetPosition(position)
      .SetOffset(settings_.bus_label_offset)
      .SetFontSize(settings_.bus_label_font_size)
      .SetFontFamily("Verdana"s)
      .SetFontWeight("bold"s);
  return stop_background;
}

svg::Text MapRenderer::MakeBusRouteNumberTitle(const std::string &name, const svg::Point &position, size_t color_count) const{
  svg::Text stop_title;
  stop_title.SetData(name)
      .SetFillColor(settings_.color_palette.at(color_count))
      .SetPosition(position)
      .SetOffset(settings_.bus_label_offset)
      .SetFontSize(settings_.bus_label_font_size)
      .SetFontFamily("Verdana"s)
      .SetFontWeight("bold"s);
  return  stop_title;
}

svg::Text MapRenderer::MakeStopNameBackground(const std::string &name, const svg::Point &position) const{
  svg::Text stop_name_background;
  stop_name_background.SetData(name)
      .SetFillColor(settings_.underlayer_color)
      .SetStrokeColor(settings_.underlayer_color)
      .SetStrokeWidth(settings_.underlayer_width)
      .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
      .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
      .SetPosition(position)
      .SetOffset(settings_.stop_label_offset)
      .SetFontSize(settings_.stop_label_font_size)
      .SetFontFamily("Verdana"s);
  return stop_name_background;
}

svg::Text MapRenderer::MakeStopNameTitle(const std::string &name, const svg::Point &position) const{
  svg::Text stop_name_title;
  stop_name_title.SetData(name)
      .SetFillColor("black"s)
      .SetPosition(position)
      .SetOffset(settings_.stop_label_offset)
      .SetFontSize(settings_.stop_label_font_size)
      .SetFontFamily("Verdana"s);
  return  stop_name_title;
}

}

}
