#include <sstream>

#include "svg.h"

namespace svg
{

    using namespace std::literals;

    void Object::Render(const RenderContext &context) const
    {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle &Circle::SetCenter(Point center)
    {
        center_ = center;
        return *this;
    }

    Circle &Circle::SetRadius(double radius)
    {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext &context) const
    {
        auto &out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        // Выводим атрибуты, унаследованные от PathProps
        RenderAttrs(context.out);
        out << "/>"sv;
    }
    // ---------- Polyline ------------------
    Polyline &Polyline::AddPoint(Point point)
    {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext &context) const
    {
        std::string tmp;
        auto &out = context.out;
        out << "<polyline points=\""sv;

        for (size_t i = 0; i < points_.size(); ++i)
        {
            out << points_[i].x << ',' << points_[i].y;
            if (i != points_.size() - 1)
            {
                out << ' ';
            }
        }
        out << "\"";
        // Выводим атрибуты, унаследованные от PathProps
        RenderAttrs(context.out);
        out << " />"sv;
    }
    // ---------- Text ------------------
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text &Text::SetPosition(Point pos)
    {
        pos_ = pos;
        return *this;
    }
    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text &Text::SetOffset(Point offset)
    {
        offset_ = offset;
        return *this;
    }
    // Задаёт размеры шрифта (атрибут font-size)
    Text &Text::SetFontSize(uint32_t size)
    {
        size_ = size;
        return *this;
    }
    // Задаёт название шрифта (атрибут font-family)
    Text &Text::SetFontFamily(std::string font_family)
    {
        font_family_ = font_family;
        return *this;
    }
    // Задаёт толщину шрифта (атрибут font-weight)
    Text &Text::SetFontWeight(std::string font_weight)
    {
        font_weight_ = font_weight;
        return *this;
    }
    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text &Text::SetData(std::string data)
    {
        for (const char &c : data)
        {
            switch (c)
            {
            case '\"':
                data_ += "&quot;"s;
                break;
            case '\'':
                data_ += "&apos;"s;
                break;
            case '<':
                data_ += "&lt;"s;
                break;
            case '>':
                data_ += "&gt;"s;
                break;
            case '&':
                data_ += "&amp;"s;
                break;
            default:
                data_ += c;
                break;
            }
        }
        data_ = data;
        return *this;
    }
    void Text::RenderObject(const RenderContext &context) const
    {
        auto &out = context.out;
        out << "<text";
        // Выводим атрибуты, унаследованные от PathProps
        RenderAttrs(context.out);
        out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" dx=\""sv << offset_.x
            << "\" dy=\""sv << offset_.y << "\" font-size=\""sv << size_;

        if (!font_family_.empty())
        {
            out << "\" font-family=\""sv << font_family_;
        }
        if (!font_weight_.empty())
        {
            out << "\" font-weight=\""sv << font_weight_;
        }

        out << '\"' << '>' << data_ << "</text>"sv;
    }

    // Добавляет в svg-документ объект-наследник svg::Object
    void Document::AddPtr(std::unique_ptr<Object> &&obj)
    {
        objects_.push_back(std::move(obj));
    }

    // Выводит в ostream svg-представление документа
    void Document::Render(std::ostream &out) const
    {
        out << "<?xml version=\"1.0\"" << " encoding=\"UTF-8\"" << " ?>"sv << std::endl
            << "<svg xmlns=\"" << "http://www.w3.org/2000/svg" << "\" version=\"1.1\">" << std::endl;
        for (const auto &obj : objects_)
        {
            obj->Render(RenderContext{out, 2, 2});
        }
        out << "</svg>"; //<< std::endl;
    }


    std::ostream &operator<<(std::ostream &out, const StrokeLineCap &slc)
    {
        switch (slc)
        {
        case StrokeLineCap::BUTT:
            out << "butt"s;
            break;
        case StrokeLineCap::ROUND:
            out << "round"s;
            break;
        case StrokeLineCap::SQUARE:
            out << "square"s;
            break;
        }
        return out;
    }
    std::ostream &operator<<(std::ostream &out, const StrokeLineJoin &slj)
    {
        switch (slj)
        {
        case StrokeLineJoin::ARCS:
            out << "arcs"s;
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel"s;
            break;
        case StrokeLineJoin::MITER:
            out << "miter"s;
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"s;
            break;
        case StrokeLineJoin::ROUND:
            out << "round"s;
            break;
        }
        return out;
    }
    /* 
    std::ostream &Color::operator<<(std::ostream &out) const
    {
        // std::ostringstream outStr;
        if (this->name != "")
        {
            out << this->name;
            return out;
        }
        else if (this->a == 1.0)
        {
            out << "rgb(" << this->r << "," << this->g << "," << this->b << ")";
        }
        else
        {
            out << "rgba(" << this->r << "," << this->g << "," << this->b << "," << this->a << ")";
        }
        // out << outStr.str();
        return out;
    }
 */
    std::ostream &operator<<(std::ostream &out, const Color color)
    {
        // std::ostringstream outStr;
        if (std::holds_alternative<std::string>(color))
        {
            out << std::get<std::string>(color);
        }
        else if (std::holds_alternative<RGB>(color))
        {
            const RGB &tmp = std::get<RGB>(color);
            out << "rgb(" << tmp.r << "," << tmp.g << "," << tmp.b << ")";
        }
        else
        {
            const RGBA &tmp = std::get<RGBA>(color);
            out << "rgba(" << tmp.r << "," << tmp.g << "," << tmp.b << "," << tmp.a << ")";
        }
        // out << outStr.str();
        return out;
    }
} // namespace svg