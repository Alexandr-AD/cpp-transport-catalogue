#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json
{

    class Node;
    // Сохраните объявления Dict и Array без изменения
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error
    {
    public:
        using runtime_error::runtime_error;
    };

    using Number = std::variant<int, double>;

    Node LoadNumber(std::istream &input);

    // Считывает содержимое строкового литерала JSON-документа
    // Функцию следует использовать после считывания открывающего символа ":
    // std::string LoadString(std::istream &input);

    class Node
    {
    public:
        /* Реализуйте Node, используя std::variant */
        using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

        Node();
        Node(std::nullptr_t);
        Node(Array array);
        Node(Dict map);
        Node(const std::string& value);
        Node(bool value);
        Node(int value);
        Node(double value);

        const Array &AsArray() const;
        const Dict &AsMap() const;
        int AsInt() const;
        double AsDouble() const;
        bool AsBool() const;
        const std::string &AsString() const;

        const Value &GetValue() const;

        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        bool operator==(const Node &other) const;
        bool operator!=(const Node &other) const;

    private:
        Value value_;
    };

    class Document
    {
    public:
        explicit Document(Node root);

        const Node &GetRoot() const;

        bool operator==(const Document &other) const;
        bool operator!=(const Document &other) const;

    private:
        Node root_;
    };

    Document Load(std::istream &input);

    void Print(const Document &doc, std::ostream &output);

    // Контекст вывода, хранит ссылку на поток вывода и текущий отступ
    struct PrintContext
    {
        std::ostream &out;
        int indent_step = 4;
        int indent = 0;

        void PrintIndent() const
        {
            for (int i = 0; i < indent; ++i)
            {
                out.put(' ');
            }
        }

        // Возвращает новый контекст вывода с увеличенным смещением
        PrintContext Indented() const
        {
            return {out, indent_step, indent_step + indent};
        }
    };

    //-------------------------------Перегрузки функции PrintValue для вывода значений------------------------------
    /* void PrintValue(std::nullptr_t, const PrintContext& ctx);
    void PrintValue(const Array &array, const PrintContext& ctx);
    void PrintValue(const Dict &dict, const PrintContext& ctx);
    void PrintValue(const bool &b, const PrintContext& ctx);
    void PrintValue(const std::string &str, const PrintContext& ctx);
    void PrintValue(const double &value, const PrintContext& ctx);
    void PrintValue(const int &value, const PrintContext& ctx); */

    void PrintNode(const Node &node, const PrintContext& ctx);

} // namespace json