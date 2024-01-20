#include "Parser.hpp"
#include "Lexer.hpp"

#include <print>
#include <unordered_map>

using namespace kaleidoscope;

using namespace std::string_literals;
using std::unique_ptr;
using std::make_unique;

using utilities::unique_ptr_cast;

static const std::unordered_map<char, int> OPERATOR_PRECEDENCE = {
    { '<', 10 },
    { '+', 20 },
    { '-', 20 },
    { '*', 40 },
};

static int get_operator_precedence(token::Operator op)
{
    if (auto it = OPERATOR_PRECEDENCE.find(op.value); it != OPERATOR_PRECEDENCE.end())
        return it->second;
    return -1;
}

unique_ptr<ast::Number> Parser::parse_number()
{
    if (not _current_token.is<token::Number>())
        throw std::runtime_error(std::format("Expected number, got {}", _current_token.to_string()));
    next_token();
    return make_unique<ast::Number>(_current_token.get<token::Number>());
}

unique_ptr<ast::Node> Parser::parse_parenthesised_expression()
{
    next_token();
    auto expr = parse_expression();
    if (not _current_token.is<token::CloseParenthesis>())
        throw std::runtime_error(std::format("Expected ')', got {}", _current_token.to_string()));
    next_token();
    return expr;
}

unique_ptr<ast::Node> Parser::parse_identifier()
{
    auto id = _current_token.get<token::Identifier>();
    next_token();
    if (not _current_token.is<token::OpenParenthesis>())
        return make_unique<ast::Identifier>(id);
    next_token();
    std::vector<unique_ptr<ast::Node>> args;
    if (not _current_token.is<token::CloseParenthesis>()) {
        while (true) {
            args.push_back(parse_expression());
            if (_current_token.is<token::CloseParenthesis>())
                break;
            if (not _current_token.is<token::Comma>())
                throw std::runtime_error(std::format("Expected ')' or ',', got {}", _current_token.to_string()));
            next_token();
        }
    }
    next_token();
    return make_unique<ast::Function::Call>(id, std::move(args));
}

unique_ptr<ast::Node> Parser::parse_primary()
{
    return utilities::match(_current_token) (
        [this](token::Number)           -> unique_ptr<ast::Node> { return parse_number(); },
        [this](token::Identifier)       -> unique_ptr<ast::Node> { return parse_identifier(); },
        [this](token::OpenParenthesis)  -> unique_ptr<ast::Node> { return parse_parenthesised_expression(); },
        [this](auto)                    -> unique_ptr<ast::Node> { throw std::runtime_error("Expected primary expression, got " + _current_token.to_string()); }
    );
}

unique_ptr<ast::Node> Parser::parse_expression()
{
    auto lhs = parse_primary();
    return parse_binary_operator(0, std::move(lhs));
}

unique_ptr<ast::Node> Parser::parse_binary_operator(int precedence, unique_ptr<ast::Node> &&lhs)
{
    while (true) {
        if (not _current_token.is<token::Operator>())
            return lhs;
        auto op = _current_token.get<token::Operator>();
        auto op_precedence = get_operator_precedence(op);
        if (op_precedence < precedence)
            return lhs;
        next_token();
        auto rhs = parse_primary();
        if (not _current_token.is<token::Operator>()) {
            lhs = make_unique<ast::BinaryOperation>(std::move(lhs), op, std::move(rhs));
            continue;
        }
        auto next_op = _current_token.get<token::Operator>();
        auto next_op_precedence = get_operator_precedence(next_op);
        if (op_precedence < next_op_precedence) {
            rhs = parse_binary_operator(op_precedence + 1, std::move(rhs));
        }
        lhs = make_unique<ast::BinaryOperation>(std::move(lhs), op, std::move(rhs));
    }
}

unique_ptr<ast::Function::Prototype> Parser::parse_prototype()
{
    if (not _current_token.is<token::Identifier>())
        throw std::runtime_error(std::format("Expected function name in prototype, got {}", _current_token.to_string()));
    auto name = _current_token.get<token::Identifier>();
    next_token();
    if (not _current_token.is<token::OpenParenthesis>())
        throw std::runtime_error(std::format("Expected '(' in prototype, got {}", _current_token.to_string()));
    next_token();
    std::vector<token::Identifier> args;
    if (not _current_token.is<token::CloseParenthesis>()) {
        while (true) {
            if (not _current_token.is<token::Identifier>())
                throw std::runtime_error(std::format("Expected argument name in prototype, got {}", _current_token.to_string()));
            args.push_back(_current_token.get<token::Identifier>());
            next_token();
            if (_current_token.is<token::CloseParenthesis>())
                break;
            if (not _current_token.is<token::Comma>())
                throw std::runtime_error(std::format("Expected ')' or ',', got {}", _current_token.to_string()));
            next_token();
        }
    }
    next_token();
    return make_unique<ast::Function::Prototype>(name, std::move(args));
}

unique_ptr<ast::Function> Parser::parse_function()
{
    next_token();
    auto prototype = parse_prototype();
    auto body = parse_expression();
    return make_unique<ast::Function>(std::move(prototype), std::move(body));
}

unique_ptr<ast::Function::Prototype> Parser::parse_extern()
{
    next_token();
    return parse_prototype();
}

unique_ptr<ast::Node> Parser::parse_top_level_expression()
{
    auto expr = parse_expression();
    return make_unique<ast::Function> (
        make_unique<ast::Function::Prototype>(token::Identifier { ""s }, std::vector<token::Identifier>()),
        std::move(expr)
    );
}

unique_ptr<ast::Node> Parser::parse()
{
    next_token();
    return utilities::match(_current_token) (
        [this](token::Def)      -> unique_ptr<ast::Node> { return parse_function(); },
        [this](token::Extern)   -> unique_ptr<ast::Node> { return parse_extern(); },
        [this](auto)            -> unique_ptr<ast::Node> { return parse_top_level_expression(); },
        [](token::InputEnd)     -> unique_ptr<ast::Node> { return nullptr; }
    );
}
