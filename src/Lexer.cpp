#include "Lexer.hpp"
#include <print>

using namespace kaleidoscope;
using namespace token;


char Lexer::next_char(int i)
{
    _previous_char = _to_be_lexed;
    _source.erase(0, i);
    _column += i;
    return _to_be_lexed = _source.front();
}

struct Token Lexer::next()
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
        if (id == "def")
            return tok(Def());
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

    switch (c) {
        case '(':
            return tok(OpenParenthesis());
        case ')':
            return tok(CloseParenthesis());
        case '+':
        case '-':
        case '*':
        case '/':
        case '<':
        case '>':
        case '=':
            return tok(Operator(c));
        case ',':
            return tok(Comma());
        case ';':
            return tok(Semicolon());
    }
    return tok(Unknown(c));
}
