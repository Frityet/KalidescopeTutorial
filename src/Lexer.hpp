#pragma once

#include <string>
#include <format>

#include "utilities.hpp"

namespace kaleidoscope
{
    namespace token
    {
        template<typename T = void>
        using Case = utilities::UnionCase<T>;
        struct InputEnd:        Case<> {};
        struct Def:             Case<> {};
        struct Extern:          Case<> {};
        struct Identifier:      Case<std::string> {};
        struct Number:          Case<double> {};
        struct OpenParenthesis: Case<> {};
        struct CloseParenthesis:Case<> {};
        struct Operator:        Case<char> {};
        struct Comma:           Case<> {};
        struct Semicolon:       Case<> {};
        struct Unknown:         Case<char> {};
    }

    struct Token : utilities::Union <
            token::InputEnd,
            token::Def,
            token::Extern,
            token::Identifier,
            token::Number,
            token::OpenParenthesis,
            token::CloseParenthesis,
            token::Operator,
            token::Comma,
            token::Semicolon,
            token::Unknown
        > {
            size_t line = 0, column = 0;

            constexpr std::string to_string() const
            {
                using namespace std::string_literals;
                using namespace token;
                return utilities::match(*this) (
                    [](InputEnd)        -> std::string { return "INPUT_END"; },
                    [](Def)             -> std::string { return "def"; },
                    [](Extern)          -> std::string { return "extern"; },
                    [](Identifier id)   -> std::string { return std::format("Identifier \"{}\"", id.value); },
                    [](Number num)      -> std::string { return std::format("Number '{}'", num.value); },
                    [](OpenParenthesis) -> std::string { return "("s; },
                    [](CloseParenthesis)-> std::string { return ")"s; },
                    [](Operator op)     -> std::string { return std::format("Operator '{}'", op.value); },
                    [](Comma)           -> std::string { return ","s; },
                    [](Semicolon)       -> std::string { return ";"s; },
                    [](Unknown unk)     -> std::string { return std::format("Unknown '{}'", unk.value); }
                ) + std::format(" at {}:{}", line, column);
            }
        };

    class Lexer {
    public:

        Lexer(const std::string &source):   _source(source) {}
        Lexer(const char *source):          _source(source) {}
        Lexer(std::string_view source):     _source(source) {}
        Lexer(std::string &&source):        _source(std::move(source)) {}
        Lexer(Lexer &&) = default;

        struct Token next();
        inline bool is_done() const { return _source.empty(); }
        inline size_t line() const { return _line; }
        inline size_t column() const { return _column; }

    private:
        char next_char(int n = 1);

        //for debug
        char _to_be_lexed, _previous_char;

        std::string _source;
        size_t _line = 1, _column = 1;
    };
}
