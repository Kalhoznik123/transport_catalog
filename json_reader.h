#pragma once

#include <iostream>
#include "request_handler.h"
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

namespace transport {

class Reader {
public:
    explicit Reader(transport::Catalogue& catalogue, renderer::MapRenderer& renderer);

    // парсит текст из потоков в JSON
    void ParseJson(std::istream& in);

    void PrintJson(std::ostream& out) const;

    svg::Color ParseColor(json::Node node) const;
private:

    renderer::RenderSettings parseVisualisationSettings() const;
    // добавляет в базу остановки
    void AddStops() const;

    // добавляет в базу расстояния между остановками
    void AddDistances() const;

    // добавляет в базу маршруты
    void AddBuses() const;

    json::Node StopStat(const json::Node& node) const;

    json::Node routeStat(const json::Node& node) const;

    json::Node mapStat(const json::Node& node) const;

    json::Document returnStat() const;


    transport::Catalogue& catalogue_;
    renderer::MapRenderer& renderer_;
    json::Array base_requests_;
    json::Array stat_requests_;
    json::Dict render_settings_;
};

} // namespace json
