#include "Lexer.hpp"
#include <print>

using namespace kaleidoscope;


char Lexer::next_char(int i)
{
    _source.erase(0, i);
    _column += i;
    return _source.front();
}

Lexer::Token Lexer::next()
{
    auto tok = [this](auto &&...args) {
        return Token { std::forward<decltype(args)>(args)..., _line, _column };
    };
    if (_source.empty())
        return tok(InputEnd());
    auto c = _source.front();
    next_char();
    if (c == '\n')
        ++_line, _column = 1;
    if (std::isspace(c))
        return next();
    if (std::isalpha(c)) {
        std::string id;
        id += c;
        while (std::isalnum(_source.front())) {
            id += _source.front();
            next_char();
        }
        if (id == "function")
            return tok(Function());
        if (id == "extern")
            return tok(Extern());
        return tok(Identifier(id));
    }
    if (std::isdigit(c)) {
        std::string num;
        num += c;
        while (std::isdigit(_source.front())) {
            num += _source.front();
            next_char();
        }
        if (_source.front() == '.') {
            num += _source.front();
            next_char();
            while (std::isdigit(_source.front())) {
                num += _source.front();
                next_char();
            }
        }
        return tok(Number(std::stod(num)));
    }
    if (c == '(')
        return tok(OpenParenthesis());
    if (c == ')')
        return tok(CloseParenthesis());
    if (c == '+' or c == '-' or c == '*' or c == '/' or c == '<' or c == '>' or c == '=')
        return Token {Operator(c)};
    return Token {Unknown(c)};
}
