#pragma once

#include "geo.h"
#include "svg.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <string>

namespace renderer
{

    inline const double EPSILON = 1e-6;
    bool IsZero(double value);

    struct offset
    {
        double dx_ = 0;
        double dy_ = 0;
    };
    
    struct RenderSettings
    {
        RenderSettings() = default;
 
        double width_ = 0;                     // ширина изображения в пикселях
        double height_ = 0;                    // высота изображения в пикселях
        double padding_ = 0;                   // отступ краёв карты от границ SVG-документа
        double line_width_ = 0;                // толщина линий, которыми рисуются автобусные маршруты
        double stop_radius_ = 0;               // радиус окружностей, которыми обозначаются остановки
        unsigned int bus_label_font_size_ = 0; // размер текста, которым написаны названия автобусных маршрутов
        offset bus_label_offset_;              // смещение надписи с названием маршрута относительно координат конечной остановки на карте
        int stop_label_font_size_ = 0;         // размер текста, которым отображаются названия остановок
        offset stop_label_offset_;             // смещение названия остановки относительно её координат на карте
        svg::Color underlayer_color_;               // цвет подложки под названиями остановок и маршрутов
        double underlayer_width_ = 0;          // толщина подложки под названиями остановок и маршрутов
        std::vector<svg::Color> color_palette_;     // цветовая палитра
    };

    class SphereProjector
    {
    public:
        // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                        double max_width, double max_height, double padding);

        // Проецирует широту и долготу в координаты внутри SVG-изображения
        svg::Point operator()(geo::Coordinates coords) const;

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

    svg::Document MapRenderer(RenderSettings &st,
                               std::vector<geo::Coordinates> &allCoords,
                               std::vector<std::pair<std::vector<std::pair<geo::Coordinates, std::string>>, std::pair<std::string, bool>>> &bus_stops_coords,
                               std::ostream &out);

    template <typename PointInputIt>
    SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                                     double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end)
        {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs)
            { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs)
            { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_))
        {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat))
        {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom)
        {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        }
        else if (width_zoom)
        {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        }
        else if (height_zoom)
        {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

}

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */