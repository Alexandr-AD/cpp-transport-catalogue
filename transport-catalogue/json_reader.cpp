#include <algorithm>
#include <cassert>
#include <iterator>
#include <map>

#include "json_reader.h"

void catalogueInput::CommandDescription::AddBusesDescr(std::vector<BusDescr> busDescr)
{
    busDescr_ = std::move(busDescr);
}
void catalogueInput::CommandDescription::AddStopsDescr(std::vector<StopDescr> stopsDescr)
{
    stopsDescr_ = std::move(stopsDescr);
}

void catalogueInput::CommandDescription::AddStopDescr(catalogueInput::StopDescr stopsDescr)
{
    stopsDescr_.push_back(stopsDescr);
}
void catalogueInput::CommandDescription::AddBusDescr(catalogueInput::BusDescr busDescr)
{
    busDescr_.push_back(busDescr);
}

std::vector<catalogueInput::StopDescr> catalogueInput::CommandDescription::GetStopsDescr()
{
    return stopsDescr_;
}
std::vector<catalogueInput::BusDescr> catalogueInput::CommandDescription::GetBusesDescr()
{
    return busDescr_;
}

const renderer::Color read_color(const json::Node &node)
{
    renderer::Color color;
    if (node.IsArray())
    {
        color = {
            node.AsArray()[0].AsInt(),
            node.AsArray()[1].AsInt(),
            node.AsArray()[2].AsInt()};
        if (node.AsArray().size() == 4)
        {
            color.a = node.AsArray()[3].AsDouble();
        }
    }
    else if (node.IsString())
    {
        color.name = node.AsString();
    }

    return color;
}

renderer::render_settings catalogueInput::read_settings(const json::Document &doc)
{
    auto inpMap = doc.GetRoot().AsMap().at("render_settings").AsMap();

    renderer::render_settings settings;

    settings.width_ = inpMap.at("width").AsDouble();
    settings.height_ = inpMap.at("height").AsDouble();
    settings.padding_ = inpMap.at("padding").AsDouble();
    settings.line_width_ = inpMap.at("line_width").AsDouble();
    settings.stop_radius_ = inpMap.at("stop_radius").AsDouble();
    settings.bus_label_font_size_ = inpMap.at("bus_label_font_size").AsInt();
    settings.bus_label_offset_ = {inpMap.at("bus_label_offset").AsArray()[0].AsDouble(), inpMap.at("bus_label_offset").AsArray()[1].AsDouble()};
    settings.stop_label_font_size_ = inpMap.at("stop_label_font_size").AsInt();
    settings.stop_label_offset_ = {inpMap.at("stop_label_offset").AsArray()[0].AsDouble(), inpMap.at("stop_label_offset").AsArray()[1].AsDouble()};
    settings.underlayer_width_ = inpMap.at("underlayer_width").AsDouble();

    settings.underlayer_color_ = read_color(inpMap.at("underlayer_color"));
    for (const auto &color : inpMap.at("color_palette").AsArray())
    {
        // const renderer::Color col = read_color(color);
        settings.color_palette_.push_back(read_color(color));
    }

    return settings;
}

// /*
//  * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
//  * а также код обработки запросов к базе и формирование массива ответов в формате JSON
//  */
