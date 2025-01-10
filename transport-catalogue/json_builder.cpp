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
                auto &current_node = const_cast<Dict &>(GetCurrentNode()->AsMap());
                current_node[str] = Node{str};
                nodes_stack_.push_back(&current_node[str]);

                return KeyContext(*this);
            }
            else
            {
                throw std::logic_error("Key() outside a dict");
            }
        }
        else
        {
            StartDict();
            Dict &current_node = const_cast<Dict &>(GetCurrentNode()->AsMap());
            current_node[str] = Node{str};
            nodes_stack_.push_back(&current_node[str]);

            return KeyContext(*this);
        }
        return KeyContext(*this);
    }
    Builder::ValueContext Builder::Value(Node::Value val)
    {
        Node tmp_node;
        switch (val.index())
        {
        case 0:
            tmp_node = nullptr;
            break;
        case 1:
            tmp_node = std::get<json::Array>(val);
            break;
        case 2:
            tmp_node = std::get<json::Dict>(val);
            break;
        case 3:
            tmp_node = std::get<bool>(val);
            break;
        case 4:
            tmp_node = std::get<int>(val);
            break;
        case 5:
            tmp_node = std::get<double>(val);
            break;
        case 6:
            tmp_node = std::get<std::string>(val);
            break;
        default:
            break;
        }
        InsertNode(tmp_node);

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
        if (root_.GetValue().index() == 0)
        {
            throw std::logic_error("Attempt to build JSON which isn't finalized");
        }
        if (!nodes_stack_.empty())
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
            if (root_.GetValue().index() == 0)
            {
                root_ = node;
                return &root_;
            }
        }
        else
        {
            if (GetCurrentNode()->IsArray())
            {
                Array &current_node = const_cast<Array &>(GetCurrentNode()->AsArray());
                current_node.push_back(node);
                return &current_node.back();
            }
            else if (GetCurrentNode()->IsString())
            {
                const std::string key = GetCurrentNode()->AsString();
                nodes_stack_.pop_back();
                if (GetCurrentNode()->IsMap())
                {
                    Dict &current_node = const_cast<Dict &>(GetCurrentNode()->AsMap());
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