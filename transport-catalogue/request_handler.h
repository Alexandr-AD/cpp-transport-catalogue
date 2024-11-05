#pragma once

#include "transport_catalogue.h"
#include "json_reader.h"

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
    void ApplyCommands([[maybe_unused]] TransportCatalogue &catalogue, catalogueInput::CommandDescription cmds) const;
};

namespace printStat
{
    json::Document PrintStats(const TransportCatalogue &tansport_catalogue, json::Array requests, const json::Document &inpDoc);
}

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)
/*
class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer);

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через
    const std::unordered_set<BusPtr>* GetBusesByStop(const std::string_view& stop_name) const;

    // Этот метод будет нужен в следующей части итогового проекта
    svg::Document RenderMap() const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
};
*/