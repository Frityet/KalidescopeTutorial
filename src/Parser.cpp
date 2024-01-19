#include "Parser.hpp"

#include <unordered_map>

using namespace kaleidoscope;

static const std::unordered_map<char, int> OPERATOR_PRECEDENCE = {
    {'<', 10},
    {'+', 20},
    {'-', 20},
    {'*', 40},
};

std::unique_ptr<Parser::Number> Parser::parse_number()
{
    auto nd = std::make_unique<Number>(_current_token.get<Lexer::Number>());
    next_token();
    return nd;
}

std::unique_ptr<Parser::Node> Parser::parse_parenthesised_expression()
{
    next_token();
    auto expr = parse_expression();
    if (not _current_token.is<Lexer::CloseParenthesis>())
        throw std::runtime_error(std::format("Expected ')' at line {}", _current_token.line));
    next_token();
    return expr;
}

std::unique_ptr<Parser::Node> Parser::parse_identifier()
{
    auto id = std::make_unique<Identifier>(_current_token.get<Lexer::Identifier>());
    next_token();
    if (not _current_token.is<Lexer::OpenParenthesis>())
        return id;
    next_token();
    std::vector<std::unique_ptr<Node>> args;
    if (not _current_token.is<Lexer::CloseParenthesis>()) {
        while (true) {
            args.push_back(parse_expression());
            if (_current_token.is<Lexer::CloseParenthesis>())
                break;
            if (not _current_token.is<Lexer::Operator>())
                throw std::runtime_error(std::format("Expected ')' or ',' at line {}", _current_token.line));
            next_token();
        }
    }
    next_token();
    return std::make_unique<Function::Call>(std::move(id), std::move(args));
}

std::unique_ptr<Parser::Node> Parser::parse_expression()
{
    auto lhs = parse();
    return parse_binary_operation(0, std::move(lhs));
}

std::unique_ptr<Parser::BinaryOperation> Parser::parse_binary_operation(int expr_prec, std::unique_ptr<Node> lhs)
{
    while (true) {
        auto op = _current_token.get<Lexer::Operator>();
        auto op_prec = OPERATOR_PRECEDENCE.at(op);
        if (op_prec < expr_prec)
            return std::make_unique<BinaryOperation>(std::move(lhs), op, nullptr);
        next_token();
        auto rhs = parse();
        if (_current_token.is<Lexer::Operator>()) {
            auto next_op = _current_token.get<Lexer::Operator>();
            auto next_op_prec = OPERATOR_PRECEDENCE.at(next_op);
            if (op_prec < next_op_prec) {
                rhs = parse_binary_operation(op_prec + 1, std::move(rhs));
            }
        }
        lhs = std::make_unique<BinaryOperation>(std::move(lhs), op, std::move(rhs));
    }
}

std::unique_ptr<Parser::Function::Prototype> Parser::parse_prototype()
{
    if (not _current_token.is<Lexer::Identifier>())
        throw std::runtime_error(std::format("Expected function name at line {}", _current_token.line));
    auto fn_name = _current_token.get<Lexer::Identifier>();
    next_token();
    if (not _current_token.is<Lexer::OpenParenthesis>())
        throw std::runtime_error(std::format("Expected '(' at line {}", _current_token.line));
    next_token();
    std::vector<Lexer::Identifier> args;
    if (not _current_token.is<Lexer::CloseParenthesis>()) {
        while (true) {
            if (not _current_token.is<Lexer::Identifier>())
                throw std::runtime_error(std::format("Expected identifier at line {}", _current_token.line));

            args.push_back(_current_token.get<Lexer::Identifier>());
            next_token();
            if (_current_token.is<Lexer::CloseParenthesis>())
                break;
            if (not _current_token.is<Lexer::Operator>())
                throw std::runtime_error(std::format("Expected ')' or ',' at line {}", _current_token.line));
            next_token();
        }
    }
    next_token();
    return std::make_unique<Function::Prototype>(fn_name, std::move(args));
}

std::unique_ptr<Parser::Function> Parser::parse_function()
{
    next_token();
    auto proto = parse_prototype();
    auto body = parse_expression();
    return std::make_unique<Function>(std::move(proto), std::move(body));
}

std::unique_ptr<Parser::Function::Prototype> Parser::parse_extern()
{
    next_token();
    return parse_prototype();
}

std::unique_ptr<Parser::Node> Parser::parse_top_level_expression()
{
    auto expr = parse_expression();
    auto proto = std::make_unique<Function::Prototype>("", std::vector<Lexer::Identifier>{});
    return std::make_unique<Function>(std::move(proto), std::move(expr));
}

std::unique_ptr<Parser::Node> Parser::parse()
{
    using NodePtr = std::unique_ptr<Node>;
    return utilities::match(next_token()) (
        [this](Lexer::Number)           -> NodePtr { return parse_number(); },
        [this](Lexer::Identifier)       -> NodePtr { return parse_identifier(); },
        [this](Lexer::OpenParenthesis)  -> NodePtr { return parse_parenthesised_expression(); },
        [this](Lexer::Extern)           -> NodePtr { return parse_extern(); },
        [this](Lexer::Function)         -> NodePtr { return parse_function(); },
        [this](Lexer::Operator op)      -> NodePtr {
            throw std::runtime_error(std::format("Unexpected operator '{}' at {}:{}", op.value, _current_token.line, _current_token.column));
        },
        [this](Lexer::Unknown uk)       -> NodePtr {
            throw std::runtime_error(std::format("Unexpected token '{}' at {}:{}", uk.value, _current_token.line));
        },
        [this](Lexer::InputEnd)         -> NodePtr {
            throw std::runtime_error(std::format("Unexpected end of input at {}:{}", _current_token.line));
        },
        [this](auto)                    -> NodePtr {
            throw std::runtime_error(std::format("Unexpected and unknown token at {}:{}", _current_token.line));
        }
    );
}
