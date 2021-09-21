#pragma once
#include <string>
#include <algorithm>
#include "svg.h"
#include "geo.h"
#include "domain.h"

namespace transport {


namespace renderer {

using namespace std::string_literals;

struct RenderSettings {
    double width /*= 1200.0*/;
    double height /*= 1200.0*/;
    double padding /*= 50.0*/;
    double line_width /*= 14.0*/;
    double stop_radius /*= 5.0*/;
    int stop_label_font_size /*= 20*/;
    int bus_label_font_size /*= 20*/;
    svg::Point bus_label_offset /*= {7.0, 15.0}*/;

    svg::Point stop_label_offset /*= {7.0, -3.0}*/;

    svg::Color underlayer_color /*= svg::Rgba(255, 255, 255, 0.85)*/;
    double underlayer_width /*= 3.0*/;
    std::vector<svg::Color> color_palette /*= {"green"s, svg::Rgb(255, 160, 0), "red"s}*/;
};

inline const double EPSILON = 1e-6;
inline bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

class SphereProjector {
public:
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width,
                    double max_height, double padding)
        : padding_(padding) {
        if (points_begin == points_end) {
            return;
        }

        const auto [left_it, right_it]
            = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
                  return lhs.lng < rhs.lng;
              });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        const auto [bottom_it, top_it]
            = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
                  return lhs.lat < rhs.lat;
              });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            zoom_coeff_ = *height_zoom;
        }
    }

    svg::Point operator()(geo::Coordinates coords) const {
        return {(coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_};
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

class MapRenderer {
public:
    // установить настройки рендеринга
    void setVisualisationSettings(RenderSettings settings);


    template <typename Stops, typename Buses, typename Points>
    void Render(svg::Document& doc, const Stops& stops, const Buses& buses, const Points& points) const;

    const RenderSettings& GetRenderSetting()const;

private:
    RenderSettings settings_;

    template <typename iterator>
    void DrawBusRouteNumber(svg::Document& doc, const SphereProjector& projector, const iterator begin, const iterator end) const;

    template <typename iterator>
    void DrawBusRouteLine(svg::Document& doc, const SphereProjector& projector, const iterator begin, const iterator end) const;

    template <typename iterator>
    void DrawStops(svg::Document& doc,const SphereProjector& projector, const iterator begin, const iterator end) const;

    template <typename iterator>
    void DrawStopsName(svg::Document& doc, const SphereProjector& projector, const iterator begin, const iterator end) const;

    svg::Polyline MakeBusRouteLine(size_t color_count)const;

    svg::Text MakeBusRouteNumberBackground(const std::string& name, const svg::Point& position)const;



    svg::Text MakeBusRouteNumberTitle(const std::string& name, const svg::Point& position,size_t color_count)const;

    svg::Text MakeStopNameBackground(const std::string& name, const svg::Point& position)const;

    svg::Text MakeStopNameTitle(const std::string& name, const svg::Point& position)const;




};
template <typename StopsContainer, typename BusesContainer, typename PointsContainer>
void MapRenderer::Render(svg::Document& doc, const StopsContainer& stops, const BusesContainer& buses, const PointsContainer& points) const {


  SphereProjector projector(points.begin(), points.end(), settings_.width, settings_.height, settings_.padding);
  DrawBusRouteLine(doc,projector,buses.begin(),buses.end());
  DrawBusRouteNumber(doc,projector,buses.begin(),buses.end());
  DrawStops(doc,projector,stops.begin(),stops.end());
  DrawStopsName(doc,projector,stops.begin(),stops.end());
}

template <typename BusIterator>
void MapRenderer::DrawBusRouteNumber(svg::Document& doc,const SphereProjector& projector, const BusIterator begin, const BusIterator end) const {

  size_t color_count = 0; // счётчик цветов
  for (auto it = begin; it!= end; ++it) {
    svg::Text start_stop_background = MakeBusRouteNumberBackground((*it)->name,projector((*it)->start_stop->coordinate));

    svg::Text start_stop_title = MakeBusRouteNumberTitle((*it)->name,projector((*it)->start_stop->coordinate),color_count);

    doc.Add(std::move(start_stop_background));
    doc.Add(std::move(start_stop_title));
    if ((*it)->end_stop!=(*it)->start_stop) {
      svg::Text finish_stop_background =  MakeBusRouteNumberBackground((*it)->name,projector((*it)->end_stop->coordinate));

      svg::Text finish_stop_count = MakeBusRouteNumberTitle((*it)->name,projector((*it)->end_stop->coordinate),color_count);;

      doc.Add(std::move(finish_stop_background));
      doc.Add(std::move(finish_stop_count));
    }
    ++color_count;
        if (color_count >= settings_.color_palette.size()) {
            color_count = 0;
        }
    }
}

template <typename BusIterator>
void MapRenderer::DrawBusRouteLine(svg::Document& doc,const SphereProjector& projector, const BusIterator begin, const BusIterator end) const {
    using namespace std::string_literals;
    size_t color_count = 0; // счётчик цветов
    for (auto it = begin; it!= end; ++it) {
        svg::Polyline bus_line = MakeBusRouteLine(color_count);

        for (const transport::Stop* stop : (*it)->stops) {
            bus_line.AddPoint(projector(stop->coordinate));
        }
        doc.Add(std::move(bus_line));
        ++color_count;
        if (color_count >= settings_.color_palette.size()) {
            color_count = 0;
        }
    }
}

template <typename StopIterator>
void MapRenderer::DrawStops(svg::Document& doc, const SphereProjector& projector, const StopIterator begin, const StopIterator end) const {
    using namespace std::string_literals;
    for (auto it = begin; it!= end; ++it) {
        svg::Circle stop_circle;
        stop_circle.SetCenter(projector((*it)->coordinate))
                .SetRadius(settings_.stop_radius)
                .SetFillColor("white"s);
        doc.Add(std::move(stop_circle));
    }
}

template <typename StopIterator>
void MapRenderer::DrawStopsName(svg::Document& doc, const SphereProjector& projector, const StopIterator begin, const StopIterator end) const {
    // Все остановки расположить В АЛФАВИТНОМ ПОРЯДКЕ!!!
    using namespace std::string_literals;

    for (auto it = begin; it != end; ++it) {
        svg::Text stop_name_background = MakeStopNameBackground((*it)->name,projector((*it)->coordinate));
        svg::Text stop_name_titile = MakeStopNameTitle((*it)->name,projector((*it)->coordinate));

        doc.Add(std::move(stop_name_background));
        doc.Add(std::move(stop_name_titile));

    }
}


} // namespace renderer
}
