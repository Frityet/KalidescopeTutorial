#pragma once

#include "Lexer.hpp"

namespace kaleidoscope
{
    namespace ast
    {
        class Node {
        public:
            virtual ~Node() = default;
        };

        class Number : public Node {
        public:
            token::Number value;

            Number(token::Number value) : value(value)
            {}
        };

        class Identifier : public Node {
        public:
            token::Identifier name;

            Identifier(token::Identifier name) : name(name)
            {}
        };

        class BinaryOperation : public Node {
        public:
            token::Operator operation;
            std::unique_ptr<Node> left_hand_side, right_hand_side;

            BinaryOperation( std::unique_ptr<Node> &&left_hand_side, token::Operator operation, std::unique_ptr<Node> &&right_hand_side)
                : operation(operation), left_hand_side(std::move(left_hand_side)), right_hand_side(std::move(right_hand_side))
            {}
        };

        class Function : public Node {
        public:
            class Call : public Node {
            public:
                token::Identifier callee;
                std::vector<std::unique_ptr<Node>> arguments;

                Call(token::Identifier callee, std::vector<std::unique_ptr<Node>> arguments)
                    : callee(callee), arguments(std::move(arguments))
                {}
            };

            class Prototype : public Node {
            public:
                token::Identifier name;
                std::vector<token::Identifier> arguments;

                Prototype(token::Identifier name, std::vector<token::Identifier> arguments)
                    : name(name), arguments(std::move(arguments))
                {}
            };

            std::unique_ptr<Prototype> prototype;
            std::unique_ptr<Node> body;

            Function(std::unique_ptr<Prototype> &&prototype, std::unique_ptr<Node> &&body)
                : prototype(std::move(prototype)), body(std::move(body))
            {}
        };
    }

    class Parser {
    public:
        Parser(Lexer &&lexer) : _lexer(std::move(lexer))
        {}

        std::unique_ptr<ast::Node> parse();

    private:
        Lexer _lexer;
        Token _current_token;

        inline const Token &next_token()
        { return _current_token = _lexer.next(); }

        std::unique_ptr<ast::Number> parse_number();
        //also parses function calls, so the real return type is std::unique_ptr<ast::Identifier | ast::Function::Call>
        std::unique_ptr<ast::Node> parse_identifier();
        std::unique_ptr<ast::Node> parse_parenthesised_expression();
        std::unique_ptr<ast::Node> parse_primary();
        std::unique_ptr<ast::Node> parse_expression();
        std::unique_ptr<ast::Node> parse_binary_operator(int precedence, std::unique_ptr<ast::Node> &&lhs);
        std::unique_ptr<ast::Node> parse_top_level_expression();

        std::unique_ptr<ast::Function::Prototype> parse_prototype();
        std::unique_ptr<ast::Function> parse_function();
        std::unique_ptr<ast::Function::Prototype> parse_extern();
    };
}
