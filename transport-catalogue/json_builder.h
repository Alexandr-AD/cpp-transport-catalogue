#pragma once

#include "json.h"

namespace json
{

    class Builder
    {
        class BaseContext;
        class ValueContext;
        class DictItemContext;
        class ArrayItemContext;
        class KeyContext;

    public:
        Builder();

        KeyContext Key(std::string);
        ValueContext Value(Node::Value);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        BaseContext EndDict();
        BaseContext EndArray();
        Node &Build();

    private:
        Node *GetCurrentNode();
        Node *InsertNode(Node);
        Node root_;
        std::vector<Node *> nodes_stack_;
        std::string current_key_;

        class BaseContext
        {
        public:
            BaseContext(Builder &builder) : builder_(builder) {}
            Node Build()
            {
                return builder_.Build();
            }
            KeyContext Key(std::string key)
            {
                return builder_.Key(std::move(key));
            }
            ValueContext Value(Node::Value val)
            {
                return builder_.Value(std::move(val));
            }
            DictItemContext StartDict()
            {
                return builder_.StartDict();
            }
            ArrayItemContext StartArray()
            {
                return builder_.StartArray();
            }
            BaseContext EndDict()
            {
                return builder_.EndDict();
            }
            BaseContext EndArray()
            {
                return builder_.EndArray();
            }

        private:
            Builder &builder_;
        };

        class ValueContext : public BaseContext
        {
        public:
            ValueContext(BaseContext base) : BaseContext(base) {}
        };

        class KeyContext : public BaseContext
        {
        public:
            KeyContext(BaseContext base) : BaseContext(base) {};

            Node Build() = delete;

            DictItemContext Value(Node::Value val)
            {
                return static_cast<DictItemContext>(BaseContext::Value(std::move(val)));
            }

            KeyContext Key(std::string key) = delete;
            BaseContext EndDict() = delete;
            BaseContext EndArray() = delete;
        };
        class DictItemContext : public BaseContext
        {
        public:
            DictItemContext(BaseContext base) : BaseContext(base) {};

            Node Build() = delete;
            DictItemContext Value(Node::Value val) = delete;
            DictItemContext StartDict() = delete;
            ArrayItemContext StartArray() = delete;
            BaseContext EndArray() = delete;
        };
        class ArrayItemContext : public BaseContext
        {
        public:
            ArrayItemContext(BaseContext base) : BaseContext(base) {};

            Node Build() = delete;

            ArrayItemContext Value(Node::Value val)
            {
                return static_cast<ArrayItemContext>(BaseContext::Value(std::move(val)));
            }
            ArrayItemContext StartArray()
            {
                return static_cast<ArrayItemContext>(BaseContext::StartArray());
            }

            KeyContext Key(std::string key) = delete;
            BaseContext EndDict() = delete;
        };
    };
} // namespace json