#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "domain.h"
#include "geo.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "json.h"

namespace catalogueInput
{
    struct StopDescr
    {
        std::string name_;
        geo::Coordinates coords_;
        std::map<std::string, int> distance_to_stops_;
    };
    struct BusDescr
    {
        std::string name_;
        std::vector<std::string> stops_;
        bool is_roundtrip_;
    };

    renderer::render_settings read_settings(const json::Document &);

    class CommandDescription
    {
    public:
        CommandDescription() = default;

        void AddStopsDescr(std::vector<StopDescr> stopsDescr);
        void AddBusesDescr(std::vector<BusDescr> busDescr);

        void AddStopDescr(StopDescr stopsDescr);
        void AddBusDescr(BusDescr busDescr);

        std::vector<StopDescr> GetStopsDescr();
        std::vector<BusDescr> GetBusesDescr();

        ~CommandDescription() = default;

    private:
        std::vector<StopDescr> stopsDescr_;
        std::vector<BusDescr> busDescr_;
    };
}
/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */