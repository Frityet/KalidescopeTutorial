#pragma once

#include "Lexer.hpp"

namespace kaleidoscope
{
    class Parser {
    public:
        class Node {
        public:
            // virtual void parse(Lexer::Token token) = 0;
            virtual ~Node() = default;
        };

        class Number : public Node {
        public:
            Lexer::Number value;

            Number(Lexer::Number value)
                : value(value)
            {}
        };

        class Identifier : public Node {
        public:
            Lexer::Identifier name;

            Identifier(Lexer::Identifier name)
                : name(name)
            {}
        };

        class BinaryOperation : public Node {
        public:
            Lexer::Operator operation;
            std::unique_ptr<Node> lhs, rhs;

            BinaryOperation( std::unique_ptr<Node> lhs, Lexer::Operator op, std::unique_ptr<Node> rhs)
                : operation(op), lhs(std::move(lhs)), rhs(std::move(rhs))
            {}
        };

        class Function : public Node {
        public:
            class Prototype : public Node {
            public:
                Lexer::Identifier name;
                std::vector<Lexer::Identifier> argument_names;

                Prototype(std::string name, std::vector<Lexer::Identifier> args)
                    : name(name), argument_names(std::move(args))
                {}
            };

            class Call : public Node {
            public:
                std::unique_ptr<Identifier> callee;
                std::vector<std::unique_ptr<Node>> arguments;

                Call(Lexer::Identifier callee, std::vector<std::unique_ptr<Node>> &&args)
                    : callee(std::make_unique<Identifier>(Identifier {callee})), arguments(std::move(args))
                {}

                Call(std::unique_ptr<Identifier> callee, std::vector<std::unique_ptr<Node>> &&args)
                    : callee(std::move(callee)), arguments(std::move(args))
                {}
            };

            std::unique_ptr<Prototype> prototype;
            std::unique_ptr<Node> body;

            Function(std::unique_ptr<Prototype> prototype, std::unique_ptr<Node> body)
                : prototype(std::move(prototype)), body(std::move(body))
            {}
        };

        Parser(Lexer lexer)
            : _lexer(lexer)
        {}

        std::unique_ptr<Node> parse();
    private:
        Lexer _lexer;
        Lexer::Token _current_token;
        inline Lexer::Token &next_token()
        { return _current_token = _lexer.next(); }
        std::unique_ptr<Number> parse_number();
        std::unique_ptr<Node> parse_identifier();
        std::unique_ptr<Node> parse_parenthesised_expression();
        std::unique_ptr<BinaryOperation> parse_binary_operation(int expr_prec, std::unique_ptr<Node> lhs);
        std::unique_ptr<Function::Prototype> parse_prototype();
        std::unique_ptr<Function> parse_function();
        std::unique_ptr<Function::Prototype> parse_extern();
        std::unique_ptr<Node> parse_top_level_expression();
        std::unique_ptr<Node> parse_expression();
        std::unique_ptr<Node> parse_primary();
    };
}
