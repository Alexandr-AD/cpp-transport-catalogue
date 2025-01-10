#include <fstream>

#include "transport_catalogue.h"
#include "request_handler.h"
#include "json.h"
#include "json_reader.h"

using namespace std;
int main()
{


    catalogueInput::InputReader reader;
    TransportCatalogue transport_catalogue;

    const json::Document doc = json::Load(cin);

    auto commandsInput = reader.ParseCommandInput(doc);
    reader.ApplyCommands(transport_catalogue, commandsInput);

    json::Document out_doc = printStat::PrintStats(transport_catalogue, doc.GetRoot().AsMap().at("stat_requests").AsArray(), doc);

    json::Print(out_doc, cout);
    /*
     * Примерная структура программы:
     *
     * Считать JSON из stdin
     * Построить на его основе JSON базу данных транспортного справочника
     * Выполнить запросы к справочнику, находящиеся в массива "stat_requests", построив JSON-массив
     * с ответами Вывести в stdout ответы в виде JSON
     */
}