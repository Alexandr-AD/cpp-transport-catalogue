#include <sstream>

#include "map_renderer.h"

namespace renderer
{
    bool IsZero(double value)
    {
        return std::abs(value) < EPSILON;
    }

    std::string toSvg(Color color)
    {
        std::ostringstream out;
        if (color.name != "")
        {
            return color.name;
        }
        else if (color.a == 1.0)
        {
            out << "rgb(" << color.r << "," << color.g << "," << color.b << ")";
            // return "rgb(" + std::to_string(color.r) + "," + std::to_string(color.g) + "," + std::to_string(color.b) + ")";
        }
        else
        {
            out << "rgba(" << color.r << "," << color.g << "," << color.b << "," << color.a << ")";
            // return "rgba(" + std::to_string(color.r) + "," + std::to_string(color.g) + "," + std::to_string(color.b) + "," +
            //        std::to_string(color.a) + ")";
        }
        std::string s_str = out.str();
        return out.str();
    }
    svg::Point SphereProjector::operator()(geo::Coordinates coords) const
    {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_};
    }

    svg::Document map_renderer(render_settings &st, std::vector<geo::Coordinates> &allCoords_sortedByStopName,
                               std::vector<std::pair<std::vector<std::pair<geo::Coordinates, std::string>>, std::pair<std::string, bool>>> &bus_stops_coords,
                               //                                                           название остановки,  название маршрута, is_roundtrip
                               std::ostream &out)
    {
        using namespace std::literals;

        svg::Document doc;
        const SphereProjector proj{
            allCoords_sortedByStopName.begin(), allCoords_sortedByStopName.end(), st.width_, st.height_, st.padding_};
        int k = 0;
        // std::vector<svg::Polyline *> lines(bus_stops_coords.size());
        for (size_t i = 0; i < bus_stops_coords.size(); ++i)
        {
            if (bus_stops_coords[i].first.empty())
            {
                continue;
            }
            svg::Polyline line;
            // if (k == st.color_palette_.size())
            // {
            //     k = 0;
            // }
            k = i % st.color_palette_.size();

            std::string col = toSvg(st.color_palette_[k]);

            line.SetStrokeColor(col);
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
            // ++k;
        }
        for (size_t i = 0; i < bus_stops_coords.size(); ++i)
        {
            svg::Text bus_name_text;
            svg::Text bus_name_text_backing;

            k = i % st.color_palette_.size();
            std::string col = toSvg(st.color_palette_[k]);

            bus_name_text.SetFillColor(col);
            bus_name_text.SetPosition(proj(bus_stops_coords[i].first[0].first));
            bus_name_text.SetOffset({st.bus_label_offset_.dx_, st.bus_label_offset_.dy_});
            bus_name_text.SetFontSize(st.bus_label_font_size_);
            bus_name_text.SetFontFamily("Verdana");
            bus_name_text.SetFontWeight("bold");
            bus_name_text.SetData(bus_stops_coords[i].second.first);

            bus_name_text_backing.SetFillColor(toSvg(st.underlayer_color_));
            bus_name_text_backing.SetStrokeColor(toSvg(st.underlayer_color_));
            bus_name_text_backing.SetStrokeWidth(st.underlayer_width_);
            bus_name_text_backing.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            bus_name_text_backing.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            bus_name_text_backing.SetPosition(proj(bus_stops_coords[i].first[0].first));
            bus_name_text_backing.SetOffset({st.bus_label_offset_.dx_, st.bus_label_offset_.dy_});
            bus_name_text_backing.SetFontSize(st.bus_label_font_size_);
            bus_name_text_backing.SetFontFamily("Verdana");
            bus_name_text_backing.SetFontWeight("bold");
            bus_name_text_backing.SetData(bus_stops_coords[i].second.first);

            doc.AddPtr(std::make_unique<svg::Text>(bus_name_text_backing));
            doc.AddPtr(std::make_unique<svg::Text>(bus_name_text));

            if (!bus_stops_coords[i].second.second &&
                (bus_stops_coords[i].first[0].first != bus_stops_coords[i].first[bus_stops_coords[i].first.size() / 2].first &&
                 bus_stops_coords[i].first[0].second != bus_stops_coords[i].first[bus_stops_coords[i].first.size() / 2].second))
            {
                svg::Text bus_name_text2;
                svg::Text bus_name_text_backing2;

                bus_name_text2.SetFillColor(col);
                bus_name_text2.SetPosition(proj(bus_stops_coords[i].first[bus_stops_coords[i].first.size() / 2].first));
                bus_name_text2.SetOffset({st.bus_label_offset_.dx_, st.bus_label_offset_.dy_});
                bus_name_text2.SetFontSize(st.bus_label_font_size_);
                bus_name_text2.SetFontFamily("Verdana");
                bus_name_text2.SetFontWeight("bold");
                bus_name_text2.SetData(bus_stops_coords[i].second.first);

                bus_name_text_backing2.SetFillColor(toSvg(st.underlayer_color_));
                bus_name_text_backing2.SetStrokeColor(toSvg(st.underlayer_color_));
                bus_name_text_backing2.SetStrokeWidth(st.underlayer_width_);
                bus_name_text_backing2.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
                bus_name_text_backing2.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                bus_name_text_backing2.SetPosition(proj(bus_stops_coords[i].first[bus_stops_coords[i].first.size() / 2].first));
                bus_name_text_backing2.SetOffset({st.bus_label_offset_.dx_, st.bus_label_offset_.dy_});
                bus_name_text_backing2.SetFontSize(st.bus_label_font_size_);
                bus_name_text_backing2.SetFontFamily("Verdana");
                bus_name_text_backing2.SetFontWeight("bold");
                bus_name_text_backing2.SetData(bus_stops_coords[i].second.first);

                doc.AddPtr(std::make_unique<svg::Text>(bus_name_text_backing2));
                doc.AddPtr(std::make_unique<svg::Text>(bus_name_text2));
            }
        }

        for (size_t i = 0; i < allCoords_sortedByStopName.size(); ++i)
        {
            svg::Circle stopSymbol;

            stopSymbol.SetCenter(proj(allCoords_sortedByStopName[i]));
            stopSymbol.SetRadius(st.stop_radius_);
            stopSymbol.SetFillColor("white");

            doc.AddPtr(std::make_unique<svg::Circle>(stopSymbol));
        }

        for (size_t j = 0; j < allCoords_sortedByStopName.size(); ++j)
        {
            svg::Text stop_label;
            svg::Text stop_label_backing;

            stop_label.SetPosition(proj(allCoords_sortedByStopName[j]));
            stop_label.SetOffset({st.stop_label_offset_.dx_, st.stop_label_offset_.dy_});
            stop_label.SetFontSize(st.stop_label_font_size_);
            stop_label.SetFontFamily("Verdana");
            stop_label.SetFillColor("black");

            stop_label_backing.SetPosition(proj(allCoords_sortedByStopName[j]));
            stop_label_backing.SetOffset({st.stop_label_offset_.dx_, st.stop_label_offset_.dy_});
            stop_label_backing.SetFontSize(st.stop_label_font_size_);
            stop_label_backing.SetFontFamily("Verdana");
            stop_label_backing.SetFillColor(toSvg(st.underlayer_color_));
            stop_label_backing.SetStrokeColor(toSvg(st.underlayer_color_));
            stop_label_backing.SetStrokeWidth(st.underlayer_width_);
            stop_label_backing.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            stop_label_backing.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

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

        doc.Render(out);
        return doc;
    }

    void renderer::Color::operator=(const Color &other)
    {
        name = other.name;
        r = other.r;
        g = other.g;
        b = other.b;
        a = other.a;
    }
}

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */