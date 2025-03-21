#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "domain.h"
#include "geo.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"
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
    struct StatRequest
    {
        int id_;
        std::string type_;
        std::string name_;
    };

    renderer::RenderSettings ReadRenderSettings(const json::Document &);
    routing::RoutingSettings ReadRoutingSettings(const json::Document &);

    class CommandDescription
    {
    public:
        CommandDescription() = default;

        void AddStopsDescr(std::vector<StopDescr> stopsDescr);
        void AddBusesDescr(std::vector<BusDescr> busDescr);

        void AddStopDescr(StopDescr stopsDescr);
        void AddBusDescr(BusDescr busDescr);

        void AddBaseRequest(StatRequest request);

        std::vector<StopDescr> GetStopsDescr() const;
        std::vector<BusDescr> GetBusesDescr() const;

        ~CommandDescription() = default;

    private:
        std::vector<StopDescr> stopsDescr_;
        std::vector<BusDescr> busDescr_;
        std::vector<StatRequest> requests_;
    };

    class InputReader
    {
    public:
        /**
         * Парсит строку в структуру CommandDescription и сохраняет результат в commands_
         */
        catalogueInput::CommandDescription ParseCommandInput(json::Document doc);

        /**
         * Наполняет данными транспортный справочник, используя команды из commands_
         */
        void ApplyCommands(TransportCatalogue &catalogue, const catalogueInput::CommandDescription &cmds) const;
    };

}

namespace printStat
{
    json::Document PrintStats(const TransportCatalogue &tansport_catalogue, json::Array requests, const json::Document &inpDoc);
}
/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */