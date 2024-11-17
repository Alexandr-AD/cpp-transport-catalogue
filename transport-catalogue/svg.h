#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <optional>
#include <vector>
#include <variant>

namespace svg
{
    struct RGB
    {
        int r = 0,
            g = 0,
            b = 0;
    };
    struct RGBA
    {
        int r = 0,
            g = 0,
            b = 0;
        double a = 0.0;
    };

    using Color = std::variant<std::string, RGB, RGBA>;

    /*
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
            // std::ostream &operator<<(std::ostream &out) const;
        };
     */
    std::ostream &operator<<(std::ostream &out, const Color color);

    // Объявив в заголовочном файле константу со спецификатором inline,
    // мы сделаем так, что она будет одной на все единицы трансляции,
    // которые подключают этот заголовок.
    // В противном случае каждая единица трансляции будет использовать свою копию этой константы
    inline const Color NoneColor{"none"};

    enum class StrokeLineCap
    {
        BUTT,
        ROUND,
        SQUARE,
    };

    std::ostream &operator<<(std::ostream &out, const StrokeLineCap &);

    enum class StrokeLineJoin
    {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };
    std::ostream &operator<<(std::ostream &out, const StrokeLineJoin &);

    template <typename Owner>
    class PathProps
    {
    public:
        Owner &SetFillColor(Color color)
        {
            fill_color_ = std::move(color);
            return AsOwner();
        }
        Owner &SetStrokeColor(Color color)
        {
            stroke_color_ = std::move(color);
            return AsOwner();
        }
        Owner &SetStrokeWidth(double width)
        {
            stroke_width_ = width;
            return AsOwner();
        }
        Owner &SetStrokeLineCap(StrokeLineCap line_cap)
        {
            stroke_line_cap_ = line_cap;
            return AsOwner();
        }
        Owner &SetStrokeLineJoin(StrokeLineJoin line_join)
        {
            stroke_line_join_ = line_join;
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        // Метод RenderAttrs выводит в поток общие для всех путей атрибуты fill и stroke
        void RenderAttrs(std::ostream &out) const
        {
            using namespace std::literals;

            if (fill_color_)
            {
                out << " fill=\""sv;
                out << *fill_color_;
                out << "\""sv;
            }
            if (stroke_color_)
            {
                out << " stroke=\""sv;
                out << *stroke_color_;
                out << "\""sv;
            }
            if (stroke_width_)
            {
                out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
            }
            if (stroke_line_cap_)
            {
                out << " stroke-linecap=\""sv << *stroke_line_cap_ << "\""sv;
            }
            if (stroke_line_join_)
            {
                out << " stroke-linejoin=\""sv << *stroke_line_join_ << "\""sv;
            }
        }

    private:
        Owner &AsOwner()
        {
            // static_cast безопасно преобразует *this к Owner&,
            // если класс Owner — наследник PathProps
            return static_cast<Owner &>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> stroke_line_cap_;
        std::optional<StrokeLineJoin> stroke_line_join_;
    };

    /*
     * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
     * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
     */
    struct RenderContext
    {
        RenderContext(std::ostream &out_)
            : out(out_)
        {
        }

        RenderContext(std::ostream &out_, int indent_step_, int indent_ = 0)
            : out(out_), indent_step(indent_step_), indent(indent_)
        {
        }

        RenderContext Indented() const
        {
            return {out, indent_step, indent + indent_step};
        }

        void RenderIndent() const
        {
            for (int i = 0; i < indent; ++i)
            {
                out.put(' ');
            }
        }

        std::ostream &out;
        int indent_step = 0;
        int indent = 0;
    };

    /*
     * Абстрактный базовый класс Object служит для унифицированного хранения
     * конкретных тегов SVG-документа
     * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
     */
    class Object
    {
    public:
        void Render(const RenderContext &context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext &context) const = 0;
    };

    class ObjectContainer
    {
    public:
        template <typename T>
        void Add(T obj);
        virtual void AddPtr(std::unique_ptr<Object> &&) = 0;
    };

    class Drawable
    {
    public:
        virtual void Draw(ObjectContainer &container) const = 0;
        virtual ~Drawable() = default;
    };

    struct Point
    {
        Point() = default;
        Point(double x_, double y_)
            : x(x_), y(y_)
        {
        }
        double x = 0;
        double y = 0;
    };

    /*
     * Класс Circle моделирует элемент <circle> для отображения круга
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
     */
    class Circle final : public Object, public PathProps<Circle>
    {
    public:
        Circle &SetCenter(Point center);
        Circle &SetRadius(double radius);

    private:
        void RenderObject(const RenderContext &context) const override;

        Point center_;
        double radius_ = 1.0;
    };

    /*
     * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
     */
    class Polyline final : public Object, public PathProps<Polyline>
    {
    public:
        // Добавляет очередную вершину к ломаной линии
        Polyline &AddPoint(Point point);

        /*
         * Прочие методы и данные, необходимые для реализации элемента <polyline>
         */

    private:
        void RenderObject(const RenderContext &context) const override;

        std::vector<Point> points_;
    };

    /*
     * Класс Text моделирует элемент <text> для отображения текста
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
     */
    class Text final : public Object, public PathProps<Text>
    {
    public:
        // Задаёт координаты опорной точки (атрибуты x и y)
        Text &SetPosition(Point pos);

        // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
        Text &SetOffset(Point offset);

        // Задаёт размеры шрифта (атрибут font-size)
        Text &SetFontSize(uint32_t size);

        // Задаёт название шрифта (атрибут font-family)
        Text &SetFontFamily(std::string font_family);

        // Задаёт толщину шрифта (атрибут font-weight)
        Text &SetFontWeight(std::string font_weight);

        // Задаёт текстовое содержимое объекта (отображается внутри тега text)
        Text &SetData(std::string data);

        // Прочие данные и методы, необходимые для реализации элемента <text>
    private:
        void RenderObject(const RenderContext &context) const override;

        Point pos_;
        Point offset_;
        uint32_t size_ = 1;
        std::string font_family_;
        std::string font_weight_;
        std::string data_;
    };

    class Document : public ObjectContainer
    {
    public:
        // Добавляет в svg-документ объект-наследник svg::Object
        void AddPtr(std::unique_ptr<Object> &&obj) override;

        // Выводит в ostream svg-представление документа
        void Render(std::ostream &out) const;

    private:
        std::vector<std::unique_ptr<Object>> objects_;
    };

    template <typename T>
    void ObjectContainer::Add(T obj)
    {
        AddPtr(std::make_unique<T>(obj));
    }

} // namespace svg