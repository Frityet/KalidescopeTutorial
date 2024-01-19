#pragma once

#include <string>
#include <format>

#include "utilities.hpp"

namespace kaleidoscope
{
    class Lexer {
    private:
        template<typename T = void>
        using Case = utilities::UnionCase<T>;
    public:
        struct InputEnd : Case<> {};
        // def is a stupid function keyword name, just gonna use `function` instead
        struct Function : Case<> {};
        struct Extern : Case<> {};
        struct Identifier : Case<std::string> {};
        struct Number : Case<double> {};
        struct OpenParenthesis : Case<> {};
        struct CloseParenthesis : Case<> {};
        struct Operator : Case<char> {};
        struct Unknown : Case<char> {};
        struct Token : utilities::Union <
            InputEnd,
            Function,
            Extern,
            Identifier,
            Number,
            OpenParenthesis,
            CloseParenthesis,
            Operator,
            Unknown
        > {
            size_t line = 0, column = 0;

            constexpr std::string to_string() const
            {
                using namespace std::string_literals;
                return utilities::match(*this) (
                    [](InputEnd)        -> std::string { return "InputEnd"; },
                    [](Function)        -> std::string { return "Function"; },
                    [](Extern)          -> std::string { return "Extern"; },
                    [](Identifier id)   -> std::string { return std::format("Identifier(\"{}\")", id.value); },
                    [](Number num)      -> std::string { return std::format("Number({})", num.value); },
                    [](OpenParenthesis) -> std::string { return "OpenParenthesis"s; },
                    [](CloseParenthesis)-> std::string { return "CloseParenthesis"s; },
                    [](Operator op)     -> std::string { return std::format("Operator('{}')", op.value); },
                    [](Unknown unk)     -> std::string { return std::format("Unknown('{}')", unk.value); }
                );
            }
        };

        Lexer(const std::string &source)
            : _source(source)
        {}

        Token next();

    private:
        char next_char(int n = 1);

        std::string _source;
        size_t _line = 1, _column = 1;
    };
}
