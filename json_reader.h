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

namespace json {

class Reader {
public:
    explicit Reader(/*RequestHandler& request_handler, */transport::Catalogue& catalogue, renderer::MapRenderer& renderer);

    // парсит текст из потоков в JSON
    void ParseJson(std::istream& in);

    void PrintJson(std::ostream& out) const;

    renderer::RenderSettings parseVisualisationSettings() const;
private:
    svg::Color ParseColor(Node node) const;

    // добавляет в базу остановки
    void AddStops() const;

    // добавляет в базу расстояния между остановками
    void AddDistances() const;

    // добавляет в базу маршруты
    void AddBuses() const;

    Node stopStat(const Node& node) const;

    Node routeStat(const Node& node) const;

    Node mapStat(const Node& node) const;

    Document returnStat() const;

    //RequestHandler& request_handler_;
    transport::Catalogue& catalogue_;
    renderer::MapRenderer& renderer_;
    Array base_requests_;
    Array stat_requests_;
    Dict render_settings_;
};

} // namespace json
