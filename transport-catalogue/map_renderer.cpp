#include <sstream>

#include "map_renderer.h"

namespace renderer
{
    void SetBusTextProperties(const RenderSettings &st, const std::string &data, int num_of_col, svg::Text &text, svg::Text &backing)
    {

        text.SetFillColor(st.color_palette_[num_of_col]);
        text.SetOffset({st.bus_label_offset_.dx_, st.bus_label_offset_.dy_});
        text.SetFontSize(st.bus_label_font_size_);
        text.SetFontFamily("Verdana");
        text.SetFontWeight("bold");
        text.SetData(data);

        backing.SetFillColor(st.underlayer_color_);
        backing.SetStrokeColor(st.underlayer_color_);
        backing.SetStrokeWidth(st.underlayer_width_);
        backing.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        backing.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        backing.SetOffset({st.bus_label_offset_.dx_, st.bus_label_offset_.dy_});
        backing.SetFontSize(st.bus_label_font_size_);
        backing.SetFontFamily("Verdana");
        backing.SetFontWeight("bold");
        backing.SetData(data);
    }
    void SetStopTextProperties(const RenderSettings &st, svg::Text &text, svg::Text &backing)
    {
        text.SetOffset({st.stop_label_offset_.dx_, st.stop_label_offset_.dy_});
        text.SetFontSize(st.stop_label_font_size_);
        text.SetFontFamily("Verdana");
        text.SetFillColor({"black"});

        backing.SetOffset({st.stop_label_offset_.dx_, st.stop_label_offset_.dy_});
        backing.SetFontSize(st.stop_label_font_size_);
        backing.SetFontFamily("Verdana");
        backing.SetFillColor(st.underlayer_color_);
        backing.SetStrokeColor(st.underlayer_color_);
        backing.SetStrokeWidth(st.underlayer_width_);
        backing.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        backing.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    }
    void RenderLines(svg::Document &doc, int &numOfColor, const SphereProjector &proj, const RenderSettings &st,
                     const std::vector<std::pair<std::vector<std::pair<geo::Coordinates, std::string>>, std::pair<std::string, bool>>> &bus_stops_coords)
    {
        using namespace std::literals;
        for (size_t i = 0; i < bus_stops_coords.size(); ++i)
        {
            if (bus_stops_coords[i].first.empty())
            {
                continue;
            }
            svg::Polyline line;
            numOfColor = i % st.color_palette_.size();

            line.SetStrokeColor(st.color_palette_[numOfColor]);
            line.SetFillColor("none"s);
            line.SetStrokeWidth(st.line_width_);
            line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);

            for (size_t j = 0; j < bus_stops_coords[i].first.size(); ++j)
            {
                svg::Point screenPoint = proj(bus_stops_coords[i].first[j].first);
                line.AddPoint(screenPoint);
            }
            doc.AddPtr(std::make_unique<svg::Polyline>(line));
        }
    }
    void RenderBusTexts(svg::Document &doc, int &numOfColor, const SphereProjector &proj, const RenderSettings &st,
                        const std::vector<std::pair<std::vector<std::pair<geo::Coordinates, std::string>>, std::pair<std::string, bool>>> &bus_stops_coords)
    {
        for (size_t i = 0; i < bus_stops_coords.size(); ++i)
        {
            svg::Text bus_name_text;
            svg::Text bus_name_text_backing;

            numOfColor = i % st.color_palette_.size();

            bus_name_text.SetPosition(proj(bus_stops_coords[i].first[0].first));
            bus_name_text_backing.SetPosition(proj(bus_stops_coords[i].first[0].first));

            SetBusTextProperties(st, bus_stops_coords[i].second.first, numOfColor, bus_name_text, bus_name_text_backing);

            doc.AddPtr(std::make_unique<svg::Text>(bus_name_text_backing));
            doc.AddPtr(std::make_unique<svg::Text>(bus_name_text));

            if (!bus_stops_coords[i].second.second &&
                (bus_stops_coords[i].first[0].first != bus_stops_coords[i].first[bus_stops_coords[i].first.size() / 2].first &&
                 bus_stops_coords[i].first[0].second != bus_stops_coords[i].first[bus_stops_coords[i].first.size() / 2].second))
            {
                svg::Text bus_name_text2;
                svg::Text bus_name_text_backing2;

                bus_name_text2.SetPosition(proj(bus_stops_coords[i].first[bus_stops_coords[i].first.size() / 2].first));
                bus_name_text_backing2.SetPosition(proj(bus_stops_coords[i].first[bus_stops_coords[i].first.size() / 2].first));

                SetBusTextProperties(st, bus_stops_coords[i].second.first, numOfColor, bus_name_text2, bus_name_text_backing2);

                doc.AddPtr(std::make_unique<svg::Text>(bus_name_text_backing2));
                doc.AddPtr(std::make_unique<svg::Text>(bus_name_text2));
            }
        }
    }

    void RenderStopSymbols(svg::Document &doc, const SphereProjector &proj, const RenderSettings &st, std::vector<geo::Coordinates> &allCoords_sortedByStopName)
    {
        for (size_t i = 0; i < allCoords_sortedByStopName.size(); ++i)
        {
            svg::Circle stopSymbol;

            stopSymbol.SetCenter(proj(allCoords_sortedByStopName[i]));
            stopSymbol.SetRadius(st.stop_radius_);
            stopSymbol.SetFillColor({"white"});

            doc.AddPtr(std::make_unique<svg::Circle>(stopSymbol));
        }
    }
    void RenderStopLabels(svg::Document &doc, const SphereProjector &proj, const RenderSettings &st, std::vector<geo::Coordinates> &allCoords_sortedByStopName,
                          const std::vector<std::pair<std::vector<std::pair<geo::Coordinates, std::string>>, std::pair<std::string, bool>>> &bus_stops_coords)
    {
        for (size_t j = 0; j < allCoords_sortedByStopName.size(); ++j)
        {
            svg::Text stop_label;
            svg::Text stop_label_backing;

            stop_label.SetPosition(proj(allCoords_sortedByStopName[j]));
            stop_label_backing.SetPosition(proj(allCoords_sortedByStopName[j]));

            SetStopTextProperties(st, stop_label, stop_label_backing);

            for (size_t i = 0; i < bus_stops_coords.size(); ++i)
            {
                auto stopName = std::find_if(bus_stops_coords[i].first.begin(), bus_stops_coords[i].first.end(),
                                             [&allCoords_sortedByStopName, &j](const std::pair<geo::Coordinates, std::string> &lhs)
                                             {
                                                 return lhs.first == allCoords_sortedByStopName[j];
                                             });
                if (stopName != bus_stops_coords[i].first.end())
                {
                    stop_label.SetData(stopName->second);
                    stop_label_backing.SetData(stopName->second);
                }
            }

            doc.AddPtr(std::make_unique<svg::Text>(stop_label_backing));
            doc.AddPtr(std::make_unique<svg::Text>(stop_label));
        }
    }

    bool IsZero(double value)
    {
        return std::abs(value) < EPSILON;
    }

    svg::Point SphereProjector::operator()(geo::Coordinates coords) const
    {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_};
    }

    svg::Document MapRenderer(RenderSettings &st, std::vector<geo::Coordinates> &allCoords_sortedByStopName,
                              std::vector<std::pair<std::vector<std::pair<geo::Coordinates, std::string>>, std::pair<std::string, bool>>> &bus_stops_coords,
                              //                                                           название остановки,  название маршрута, is_roundtrip
                              std::ostream &out)
    {
        using namespace std::literals;

        svg::Document doc;
        const SphereProjector proj{
            allCoords_sortedByStopName.begin(), allCoords_sortedByStopName.end(), st.width_, st.height_, st.padding_};
        int k = 0;
        RenderLines(doc, k, proj, st, bus_stops_coords);
        RenderBusTexts(doc, k, proj, st, bus_stops_coords);
        RenderStopSymbols(doc, proj, st, allCoords_sortedByStopName);
        RenderStopLabels(doc, proj, st, allCoords_sortedByStopName, bus_stops_coords);

        doc.Render(out);
        return doc;
    }
}

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */