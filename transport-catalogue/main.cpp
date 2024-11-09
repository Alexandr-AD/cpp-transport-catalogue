#include <fstream>

#include "transport_catalogue.h"
#include "request_handler.h"
#include "json.h"
#include "json_reader.h"

using namespace std;
int main()
{

    {
        ifstream input("input1.json");
        ofstream output("out1.json");

        catalogueInput::InputReader reader;
        TransportCatalogue transport_catalogue;

        const json::Document doc = json::Load(input);

        auto commandsInput = reader.ParseCommandInput(doc);
        reader.ApplyCommands(transport_catalogue, commandsInput);

        json::Document out_doc = printStat::PrintStats(transport_catalogue, doc.GetRoot().AsMap().at("stat_requests").AsArray(), doc);

        json::Print(out_doc, output);
    }

    /* {
        ifstream input("input2.json");
        ofstream output("out2.json");

        InputReader reader;
        TransportCatalogue transport_catalogue;

        const json::Document doc = json::Load(input);

        auto commandsInput = reader.ParseCommandInput(doc);
        reader.ApplyCommands(transport_catalogue, commandsInput);

        json::Document out_doc = printStat::PrintStats(transport_catalogue, doc.GetRoot().AsMap().at("stat_requests").AsArray(), doc);

        json::Print(out_doc, output);
    }

    {
        ifstream input("input3.json");
        ofstream output("out3.json");

        InputReader reader;
        TransportCatalogue transport_catalogue;

        const json::Document doc = json::Load(input);

        auto commandsInput = reader.ParseCommandInput(doc);
        reader.ApplyCommands(transport_catalogue, commandsInput);

        json::Document out_doc = printStat::PrintStats(transport_catalogue, doc.GetRoot().AsMap().at("stat_requests").AsArray(), doc);

        json::Print(out_doc, output);
    } */
    // auto settings = catalogueInput::read_settings(doc);
    // auto allCoords = transport_catalogue.GetStopsCoords();
    // auto busCoords = transport_catalogue.GetBusStopsCoords();
    // renderer::MapRenderer(settings, allCoords, busCoords, output);

    // auto commandsRequest = reader.ParseCommandRequest(input);
    // reader.ApplyCommands(transport_catalogue, commandsRequest);

    /*
     * Примерная структура программы:
     *
     * Считать JSON из stdin
     * Построить на его основе JSON базу данных транспортного справочника
     * Выполнить запросы к справочнику, находящиеся в массива "stat_requests", построив JSON-массив
     * с ответами Вывести в stdout ответы в виде JSON
     */
}