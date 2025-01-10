#pragma once

#include "json.h"

namespace json
{

    class Builder
    {
        class BaseContext;
        class ValueContext;
        class KeyContext;
        class DictValueContext;
        class DictItemContext;
        class ArrayItemContext;
        class ArrayValueContext;

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
                return builder_.Key(key);
            }
            ValueContext Value(Node::Value val)
            {
                return builder_.Value(val);
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

            DictValueContext Value(Node::Value val)
            {
                return static_cast<DictValueContext>(BaseContext::Value(val));
            }

            KeyContext Key(std::string key) = delete;
            BaseContext EndDict() = delete;
            BaseContext EndArray() = delete;
        };
        class DictValueContext : public BaseContext
        {
        public:
            DictValueContext(BaseContext base) : BaseContext(base) {};

            Node Build() = delete;

            DictValueContext Value(Node::Value val) = delete;
            DictItemContext StartDict() = delete;
            ArrayItemContext StartArray() = delete;
            BaseContext EndArray() = delete;
        };
        class DictItemContext : public BaseContext
        {
        public:
            DictItemContext(BaseContext base) : BaseContext(base) {};

            Node Build() = delete;
            DictValueContext Value(Node::Value val) = delete;
            DictItemContext StartDict() = delete;
            ArrayItemContext StartArray() = delete;
            BaseContext EndArray() = delete;
        };
        class ArrayItemContext : public BaseContext
        {
        public:
            ArrayItemContext(BaseContext base) : BaseContext(base) {};

            Node Build() = delete;

            ArrayValueContext Value(Node::Value val)
            {
                return static_cast<ArrayValueContext>(BaseContext::Value(val));
            }
            ArrayItemContext StartArray()
            {
                return static_cast<ArrayItemContext>(BaseContext::StartArray());
            }

            KeyContext Key(std::string key) = delete;
            BaseContext EndDict() = delete;
        };
        class ArrayValueContext : public ValueContext
        {
        public:
            ArrayValueContext(BaseContext base) : ValueContext(base) {}

            ArrayValueContext Value(Node::Value val)
            {
                return static_cast<ArrayValueContext>(BaseContext::Value(val));
            }

            Node Build() = delete;
            KeyContext Key(std::string key) = delete;
            BaseContext EndDict() = delete;
        };
    };
} // namespace json