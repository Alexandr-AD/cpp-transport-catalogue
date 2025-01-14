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
                current_key_ = std::move(str);
            }
            else
            {
                throw std::logic_error("Key() outside a dict");
            }
        }
        else
        {
            StartDict();
            current_key_ = std::move(str);
            // подскажите, пожалуйста, я здесь пробовал сразу выбрасывать исключение, в таком случае не проходит ни один тест,
            // тренажер пишет, что решение упало и содержание этого исключения. Пробовал по всякому, проверял, без startdict в этом месте тесты не проходит.
            // Исключение из GetCurrentNode здесь не выбрасывается, потому что добавляется словарь. Этот else у меня убрать не получается, без него совсем не работает, как правильно это исправить?
            // Написал в пачку с этим же вопросом вам
            // throw std::logic_error("_______________Key() outside a dict_____________");
        }
        return KeyContext(*this);
    }
    Builder::ValueContext Builder::Value(Node::Value val)
    {
        Node tmp_node(std::move(val));

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
                Array &current_node = GetCurrentNode()->AsArray();
                current_node.push_back(node);
                return &current_node.back();
            }
            else if (!current_key_.empty())
            {
                if (GetCurrentNode()->IsMap())
                {
                    const auto key = std::move(current_key_);
                    Dict &current_node = GetCurrentNode()->AsMap();
                    current_node.insert({key, node});
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