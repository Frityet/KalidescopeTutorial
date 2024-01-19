#pragma once

#include <variant>
#include <string_view>

namespace kaleidoscope::utilities
{
    template<class ...Ts>
    struct overload : Ts... {
        using Ts::operator()...;
    };

    struct UnionCaseBase {};
    template<typename T = void>
    struct UnionCase : UnionCaseBase {
        using type = T;
        T value;

        UnionCase(T value)
            : value(value)
        {}

        template<typename U>
        constexpr bool is() const
        { return std::is_same_v<T, U>; }

        template<typename U>
        constexpr U as() const
        { return static_cast<U>(value); }

        constexpr operator T() const
        { return value; }
    };

    template<>
    struct UnionCase<> : UnionCaseBase {
        template<typename U>
        constexpr bool is() const
        { return false; }

        template<typename U>
        constexpr U as() const
        { return {}; }
    };

    template<const char *...T>
    struct StringUnion : UnionCaseBase {
        std::string_view value;

        constexpr StringUnion(std::string_view value)
            : value(value)
        {}

        template<const char *U>
        constexpr bool is() const
        { return value == U; }

        template<const char *U>
        constexpr operator bool() const
        { return is<U>(); }
    };


    template<typename T>
    concept HasValue = requires (T t) {
        { T::value } -> std::convertible_to<typename T::type>;
    };

    template<typename ...T>
    struct Union : std::variant<T...> {
        using std::variant<T...>::variant;

        template<typename U>
        constexpr bool is() const
        { return std::holds_alternative<U>(*this); }

        template<typename U>
        constexpr U &get()
        { return std::get<U>(*this); }

        template<typename U>
        constexpr const U &get() const
        { return std::get<U>(*this); }

        template<typename U> requires HasValue<U>
        constexpr auto &value()
        { return this->get<U>().value; }

        template<typename U> requires HasValue<U>
        constexpr const auto &value() const
        { return this->get<U>().value; }

        template<typename U> requires HasValue<U>
        constexpr operator auto() const
        { return this->get<U>().value; }
    };

    template<typename ...Variant>
    static constexpr auto match(Variant &&...var)
    {
        return [&var...]<typename... T>(T &&...matchers) -> decltype(auto) {
            return std::visit(overload{std::forward<T>(matchers)...}, std::forward<Variant>(var)...);
        };
    }

    #define $CASE_IMPL(...) { return __VA_ARGS__; }
    #define $case(T) [&](T) $CASE_IMPL
    #define $default(...) $case(auto) $CASE_IMPL(__VA_ARGS__)
}
