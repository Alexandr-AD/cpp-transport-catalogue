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
    struct Color
    {

        std::string name; // имя цвета в виде строки
        int r = 0,
            g = 0,
            b = 0;      // компоненты цвета в формате svg::Rgb
        double a = 0.0; // opacity в формате svg::Rgba

        Color(const Color &other)
        {
            name = other.name;
            r = other.r;
            g = other.g;
            b = other.b;
            a = other.a;
        }

        Color() = default;

        // Конструктор для инициализации цвета из строки
        Color(std::string name_) : name(name_), r(0), g(0), b(0), a(1.0) {}

        // Конструктор для инициализации цвета из массива из трёх целых чисел
        Color(int r_, int g_, int b_) : name(""), r(r_), g(g_), b(b_), a(1.0) {}

        // Конструктор для инициализации цвета из массива из четырёх элементов
        Color(int r_, int g_, int b_, float a_) : name(""), r(r_), g(g_), b(b_), a(a_) {}

        void operator=(const Color &other);
    };
    struct render_settings
    {
        render_settings() = default;
        /*  render_settings(double width,
                         double height,
                         double padding,
                         int line_width,
                         double stop_radius,
                         int bus_label_font_size,
                         offset bus_label_offset,
                         int stop_label_font_size,
                         offset stop_label_offset,
                         Color underlayer_color,
                         double underlayer_width,
                         std::vector<Color> color_palette)
         {
             width_ = width;
             height_ = height;
             padding_ = padding;
             line_width_ = line_width;
             stop_radius_ = stop_radius;
             bus_label_font_size_ = bus_label_font_size;
             bus_label_offset_ = bus_label_offset;
             stop_label_font_size_ = stop_label_font_size;
             stop_label_offset_ = stop_label_offset;
             underlayer_color_ = underlayer_color;
             underlayer_width_ = underlayer_width;
             color_palette_ = color_palette_;
         } */
        render_settings(const render_settings &st)
        {
            width_ = st.width_;
            height_ = st.height_;
            padding_ = st.padding_;
            line_width_ = st.line_width_;
            stop_radius_ = st.stop_radius_;
            bus_label_font_size_ = st.bus_label_font_size_;
            bus_label_offset_ = st.bus_label_offset_;
            stop_label_font_size_ = st.stop_label_font_size_;
            stop_label_offset_ = st.stop_label_offset_;
            underlayer_color_ = st.underlayer_color_;
            underlayer_width_ = st.underlayer_width_;
            color_palette_ = st.color_palette_;
        }

        double width_ = 0;                 // ширина изображения в пикселях
        double height_ = 0;                // высота изображения в пикселях
        double padding_ = 0;               // отступ краёв карты от границ SVG-документа
        double line_width_ = 0;            // толщина линий, которыми рисуются автобусные маршруты
        double stop_radius_ = 0;           // радиус окружностей, которыми обозначаются остановки
        unsigned int bus_label_font_size_ = 0;      // размер текста, которым написаны названия автобусных маршрутов
        offset bus_label_offset_;          // смещение надписи с названием маршрута относительно координат конечной остановки на карте
        int stop_label_font_size_ = 0;     // размер текста, которым отображаются названия остановок
        offset stop_label_offset_;         // смещение названия остановки относительно её координат на карте
        Color underlayer_color_;           // цвет подложки под названиями остановок и маршрутов
        double underlayer_width_ = 0;      // толщина подложки под названиями остановок и маршрутов
        std::vector<Color> color_palette_; // цветовая палитра
    };

    // Функция для вывода цвета в SVG формате
    std::string toSvg(Color color);

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

    svg::Document map_renderer(render_settings &st,
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