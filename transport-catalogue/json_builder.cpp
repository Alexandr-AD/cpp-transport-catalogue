#include <sstream>

#include "json_builder.h"

namespace json
{
    Builder::Builder()
    {
        root_ = nullptr;
    }
    Builder::KeyContext Builder::Key(std::string str)
    {
        if (!nodes_stack_.empty())
        {
            if (GetCurrentNode()->IsMap())
            {
                auto &current_node = GetCurrentNode()->ChangeMap();
                current_node[str] = Node{str};              // ключ временно записывается в стек, удаляется при записи значения
                nodes_stack_.push_back(&current_node[str]); // не понял насчет поиска - тут же не поиск, а запись в вектор?
            }
            else
            {
                throw std::logic_error("Key() outside a dict");
            }
        }
        else
        {
            StartDict();                                        // не понимаю, почему некорректно и какая ошибочная ситуация? Ключ начинает словарь
            Dict &current_node = GetCurrentNode()->ChangeMap(); // не понял насчет исключений. ниже же нигде не выбрасываются исключения?
            current_node[str] = Node{str};                      // да и все тесты в тренажере оно прошло, почему тут должна быть ошибочная ситуация?
            nodes_stack_.push_back(&current_node[str]);
        }
        return KeyContext(*this);
    }
    Builder::ValueContext Builder::Value(Node::Value val)
    {
        Node tmp_node(val);

        InsertNode(std::move(tmp_node));

        return ValueContext(*this);
    }
    Builder::DictItemContext Builder::StartDict()
    {
        nodes_stack_.push_back(InsertNode(Dict{}));

        return DictItemContext(*this);
    }
    Builder::ArrayItemContext Builder::StartArray()
    {
        nodes_stack_.push_back(InsertNode(Array{}));

        return ArrayItemContext(*this);
    }
    Builder::BaseContext Builder::EndDict()
    {
        if (!nodes_stack_.empty())
        {
            if (GetCurrentNode()->IsMap())
            {
                nodes_stack_.pop_back();

                return BaseContext(*this);
            }
        }
        throw std::logic_error("EndDict() outside a dict");
    }
    Builder::BaseContext Builder::EndArray()
    {
        if (!nodes_stack_.empty())
        {
            if (GetCurrentNode()->IsArray())
            {
                nodes_stack_.pop_back();

                return BaseContext(*this);
            }
        }
        throw std::logic_error("EndDict() outside a dict");
    }
    Node &Builder::Build()
    {
        if (std::holds_alternative<std::nullptr_t>(root_.GetValue()) || !nodes_stack_.empty())
        {
            throw std::logic_error("Attempt to build JSON which isn't finalized");
        }

        return root_;
    }

    Node *Builder::GetCurrentNode()
    {
        if (nodes_stack_.empty())
        {
            throw std::logic_error("Attempt to change finalized JSON");
        }
        return nodes_stack_.back();
    }
    Node *Builder::InsertNode(Node node)
    {
        if (nodes_stack_.empty())
        {
            if (std::holds_alternative<std::nullptr_t>(root_.GetValue()))
            {
                root_ = node;
                return &root_;
            }
        }
        else
        {
            if (GetCurrentNode()->IsArray())
            {
                Array &current_node = GetCurrentNode()->ChangeArray();
                current_node.push_back(node);
                return &current_node.back();
            }
            else if (GetCurrentNode()->IsString())
            {
                const std::string key = GetCurrentNode()->AsString(); // ключ хранится в стеке временно, в следующей же строке он удаляется из стека
                nodes_stack_.pop_back();                              // это работает как флаг для записи значения по ключу
                if (GetCurrentNode()->IsMap())
                {
                    Dict &current_node = GetCurrentNode()->ChangeMap();
                    current_node[key] = node;
                    return &current_node[key];
                }
                else
                {
                    throw std::logic_error("Key is out of dict");
                }
            }
            else
            {
                throw std::logic_error("Incorrect format");
            }
        }
        throw std::logic_error("Incorrect format");
    }
} // namespace json