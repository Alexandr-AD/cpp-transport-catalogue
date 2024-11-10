#include "json.h"

using namespace std;

namespace json
{

    namespace
    {
        using namespace std::literals;

        Node LoadNode(istream &input);
        Node LoadString(std::istream &input);

        std::string LoadLiteral(std::istream &input)
        {
            std::string s;
            while (std::isalpha(input.peek()))
            {
                s.push_back(static_cast<char>(input.get()));
            }
            return s;
        }

        Node LoadArray(istream &input)
        {
            Array result;

            for (char c; input >> c && c != ']';)
            {
                if (c != ',')
                {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            if (!input)
            {
                throw ParsingError("Array parsing error"s);
            }

            return Node(move(result));
        }

        Node LoadNull(istream &input)
        {
            if (auto literal = LoadLiteral(input); literal == "null"sv)
            {
                return Node{nullptr};
            }
            else
            {
                throw ParsingError("Failed to parse '"s + literal + "' as null"s);
            }
        }

        Node LoadBool(istream &input)
        {
            const string str = LoadLiteral(input);
            if (str == "true"s)
            {
                return Node(true);
            }
            else if (str == "false"s)
            {
                return Node(false);
            }
            else
            {
                throw ParsingError("Bool error");
            }
        }

        /*         Node LoadString(istream &input)
                {
                    string line = json::LoadString(input);
                    return Node(move(line));
                } */

        Node LoadString(std::istream &input)
        {
            // using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true)
            {
                if (it == end)
                {
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"')
                {
                    ++it;
                    break;
                }
                else if (ch == '\\')
                {
                    ++it;
                    if (it == end)
                    {
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    switch (escaped_char)
                    {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r')
                {
                    throw ParsingError("Unexpected end of line"s);
                }
                else
                {
                    s.push_back(ch);
                }
                ++it;
            }

            return Node(std::move(s));
        }

        Node LoadDict(istream &input)
        {
            Dict result;

            for (char c; input >> c && c != '}';)
            {
                if (c == '"')
                {
                    std::string key = LoadString(input).AsString();
                    if (input >> c && c == ':')
                    {
                        if (result.find(key) != result.end())
                        {
                            throw ParsingError("Duplicate key '"s + key + "' have been found");
                        }
                        result.emplace(std::move(key), LoadNode(input));
                    }
                    else
                    {
                        throw ParsingError(": is expected but '"s + c + "' has been found"s);
                    }
                }
                else if (c != ',')
                {
                    throw ParsingError(R"(',' is expected but ')"s + c + "' has been found"s);
                }
            }
            if (!input)
            {
                throw ParsingError("Dictionary parsing error"s);
            }
            return Node(std::move(result));
        }

        Node LoadNode(istream &input)
        {
            char c;
            if (!(input >> c))
            {
                throw ParsingError("Unexpected EOF"s);
            }
            switch (c)
            {
            case '[':
                return LoadArray(input);
            case '{':
                return LoadDict(input);
            case '"':
                return LoadString(input);
            case 't':
                // Атрибут [[fallthrough]] (провалиться) ничего не делает, и является
                // подсказкой компилятору и человеку, что здесь программист явно задумывал
                // разрешить переход к инструкции следующей ветки case, а не случайно забыл
                // написать break, return или throw.
                // В данном случае, встретив t или f, переходим к попытке парсинга
                // литералов true либо false
                [[fallthrough]];
            case 'f':
                input.putback(c);
                return LoadBool(input);
            case 'n':
                input.putback(c);
                return LoadNull(input);
            default:
                input.putback(c);
                return LoadNumber(input);
            }
        }

    } // namespace
    //---------------------конструкторы--------------------------------
    Node::Node()
        : value_(nullptr)
    {
    }
    Node::Node(std::nullptr_t)
        : value_(nullptr)
    {
    }
    //-----------------------------возврат значения конкретного типа------------------------
    const Array &Node::AsArray() const
    {
        if (!IsArray())
        {
            throw logic_error("Not array");
        }
        return get<Array>(value_);
    }

    const Dict &Node::AsMap() const
    {
        if (!IsMap())
        {
            throw logic_error("Not map");
        }
        return get<Dict>(value_);
    }

    int Node::AsInt() const
    {
        if (!IsInt())
        {
            throw logic_error("Not int");
        }

        return get<int>(value_);
    }

    double Node::AsDouble() const
    {
        if (!IsDouble())
        {
            throw logic_error("Not double");
        }
        if (IsInt())
        {
            return get<int>(value_);
        }
        return get<double>(value_);
    }
    bool Node::AsBool() const
    {
        if (!IsBool())
        {
            throw logic_error("Not bool");
        }

        return get<bool>(value_);
    }

    const string &Node::AsString() const
    {
        if (!IsString())
        {
            throw logic_error("Not string");
        }
        return get<string>(value_);
    }
    //-----------------------------возврат значения конкретного типа------------------------

    const Node::Value &Node::GetValue() const
    {
        return value_;
    }

    //-----------------------------проверка типа------------------------
    bool Node::IsNull() const
    {
        return std::holds_alternative<nullptr_t>(value_);
    }
    bool Node::IsArray() const
    {
        return std::holds_alternative<Array>(value_);
    }
    bool Node::IsMap() const
    {
        return std::holds_alternative<Dict>(value_);
    }
    bool Node::IsBool() const
    {
        return std::holds_alternative<bool>(value_);
    }
    bool Node::IsInt() const
    {
        return std::holds_alternative<int>(value_);
    }
    bool Node::IsDouble() const
    {
        return std::holds_alternative<double>(value_) || std::holds_alternative<int>(value_);
    }
    bool Node::IsPureDouble() const
    {
        return std::holds_alternative<double>(value_);
    }
    bool Node::IsString() const
    {
        return std::holds_alternative<string>(value_);
    }
    //-----------------------------проверка типа------------------------

    Node LoadNumber(std::istream &input)
    {
        // using namespace std::literals;

        std::string parsed_num;

        // Считывает в parsed_num очередной символ из input
        auto read_char = [&parsed_num, &input]
        {
            parsed_num += static_cast<char>(input.get());
            if (!input)
            {
                throw ParsingError("Failed to read number from stream"s);
            }
        };

        // Считывает одну или более цифр в parsed_num из input
        auto read_digits = [&input, read_char]
        {
            if (!std::isdigit(input.peek()))
            {
                throw ParsingError("A digit is expected"s);
            }
            while (std::isdigit(input.peek()))
            {
                read_char();
            }
        };

        if (input.peek() == '-')
        {
            read_char();
        }
        // Парсим целую часть числа
        if (input.peek() == '0')
        {
            read_char();
            // После 0 в JSON не могут идти другие цифры
        }
        else
        {
            read_digits();
        }

        bool is_int = true;
        // Парсим дробную часть числа
        if (input.peek() == '.')
        {
            read_char();
            read_digits();
            is_int = false;
        }

        // Парсим экспоненциальную часть числа
        if (int ch = input.peek(); ch == 'e' || ch == 'E')
        {
            read_char();
            if (ch = input.peek(); ch == '+' || ch == '-')
            {
                read_char();
            }
            read_digits();
            is_int = false;
        }

        try
        {
            if (is_int)
            {
                // Сначала пробуем преобразовать строку в int
                try
                {
                    return std::stoi(parsed_num);
                }
                catch (...)
                {
                    // В случае неудачи, например, при переполнении,
                    // код ниже попробует преобразовать строку в double
                }
            }
            return std::stod(parsed_num);
        }
        catch (...)
        {
            throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
        }
    }
    //-----------------------------Сравнение Node------------------------

    bool CompareArray(const Array &nodearray1, const Array &nodearray2)
    {
        if (nodearray1.size() != nodearray2.size())
        {
            return false;
        }
        for (size_t i = 0; i < nodearray1.size(); ++i)
        {
            if (nodearray1[i] != nodearray2[i])
            {
                return false;
            }
        }
        return true;
    }

    bool Node::operator==(const Node &other) const
    {
        if (value_.index() != other.value_.index())
        {
            return false;
        }
        switch (value_.index())
        {
        case 0:
            return get<nullptr_t>(value_) == get<nullptr_t>(other.value_);
            break;
        case 1:
            return get<Array>(value_) == get<Array>(other.value_);
            break;
        case 2:
            return get<Dict>(value_) == get<Dict>(other.value_);
            break;
        case 3:
            return get<bool>(value_) == get<bool>(other.value_);
            break;
        case 4:
            return get<int>(value_) == get<int>(other.value_);
            break;
        case 5:
            return get<double>(value_) == get<double>(other.value_);
            break;
        case 6:
            return get<string>(value_) == get<string>(other.value_);
            break;
        }
        return true;
    }
    bool Node::operator!=(const Node &other) const
    {
        return !(*this == other);
    }

    Document::Document(Node root)
        : root_(move(root))
    {
    }

    const Node &Document::GetRoot() const
    {
        return root_;
    }

    bool Document::operator==(const Document &other) const
    {
        if (root_ == other.root_)
        {
            return true;
        }
        return false;
    }
    bool Document::operator!=(const Document &other) const
    {
        return !(*this == other);
    }

    Document Load(istream &input)
    {
        return Document{LoadNode(input)};
    }

    void Print(const Document &doc, std::ostream &output)
    {
        PrintNode(doc.GetRoot(), PrintContext{output});
    }

    //-------------------------------Перегрузки функции PrintValue для вывода значений------------------------------
    void PrintValue(std::nullptr_t, const PrintContext &ctx)
    {
        ctx.out << "null"sv;
    }
    void PrintValue(const Array &nodes, const PrintContext &ctx)
    {
        std::ostream &out = ctx.out;
        out << "[\n"sv;
        bool first = true;
        auto inner_ctx = ctx.Indented();
        for (const Node &node : nodes)
        {
            if (first)
            {
                first = false;
            }
            else
            {
                out << ",\n"sv;
            }
            inner_ctx.PrintIndent();
            PrintNode(node, inner_ctx);
        }
        out.put('\n');
        ctx.PrintIndent();
        out.put(']');
    }

    void PrintString(const std::string &value, std::ostream &out)
    {
        out.put('"');
        for (const char c : value)
        {
            switch (c)
            {
            case '\r':
                out << "\\r"sv;
                break;
            case '\n':
                out << "\\n"sv;
                break;
            case '\t':
                out << "\\t"sv;
                break;
            case '"':
                // Символы " и \ выводятся как \" или \\, соответственно
                [[fallthrough]];
            case '\\':
                out.put('\\');
                [[fallthrough]];
            default:
                out.put(c);
                break;
            }
        }
        out.put('"');
    }

    void PrintValue(const Dict &nodes, const PrintContext &ctx)
    {
        std::ostream &out = ctx.out;
        out << "{\n"sv;
        bool first = true;
        auto inner_ctx = ctx.Indented();
        for (const auto &[key, node] : nodes)
        {
            if (first)
            {
                first = false;
            }
            else
            {
                out << ",\n"sv;
            }
            inner_ctx.PrintIndent();
            PrintString(key, ctx.out);
            out << ": "sv;
            PrintNode(node, inner_ctx);
        }
        out.put('\n');
        ctx.PrintIndent();
        out.put('}');
    }
    void PrintValue(const bool &value, const PrintContext &ctx)
    {
        ctx.out << (value ? "true"sv : "false"sv);
    }

    void PrintValue(const std::string &str, const PrintContext &ctx)
    {
        PrintString(str, ctx.out);
    }
    template <typename Value>
    void PrintValue(const Value &value, const PrintContext &ctx)
    {
        ctx.out << value;
    }
    //-------------------------------Перегрузки функции PrintValue для вывода значений------------------------------

    void PrintNode(const Node &node, const PrintContext &ctx)
    {
        std::visit(
            [&ctx](const auto &value)
            { PrintValue(value, ctx); },
            node.GetValue());
    }

} // namespace json