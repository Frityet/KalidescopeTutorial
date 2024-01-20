#pragma once

#include <variant>
#include <string>
#include <vector>
#include <cxxabi.h>
#include <memory>
#include <format>
#include <print>
namespace kaleidoscope::utilities
{
    struct UnionCaseBase {};
    template<typename T = void>
    struct UnionCase : UnionCaseBase {
        using type = T;
        T value;

        constexpr UnionCase(T value)
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

        constexpr operator T &()
        { return value; }

        constexpr operator const T &() const
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

    template<class ...Ts>
    struct overload : Ts... {
        using Ts::operator()...;
    };

    template<typename ...Variant>
    static constexpr auto match(Variant &&...var)
    {
        return [&var...]<typename... T>(T &&...matchers) -> decltype(auto) {
            return std::visit(overload{std::forward<T>(matchers)...}, std::forward<Variant>(var)...);
        };
    }

    static std::string type_name(const std::type_info &ti)
    {
        static char tmp_buf[1024] = {0};
        int status = 0;
        auto length = sizeof(tmp_buf);
        abi::__cxa_demangle(ti.name(), tmp_buf, &length, &status);
        if (status != 0)
            throw std::runtime_error(std::format("Failed to demangle type name: {}", status));
        return std::string(tmp_buf, length);
    }

    template <typename TTo, typename TFrom> requires std::is_base_of_v<TTo, TFrom> or std::is_base_of_v<TFrom, TTo> or std::is_same_v<TTo, TFrom>
    static constexpr std::unique_ptr<TTo> unique_ptr_cast(std::unique_ptr<TFrom> &&from)
    {
        auto fromptr = from.release();
        auto toptr = dynamic_cast<TTo *>(fromptr);
        if (toptr == nullptr)
            throw std::runtime_error(std::format("Failed to cast from {} (claimed to be {}) to {}", type_name(typeid(*fromptr)), type_name(typeid(TFrom)), type_name(typeid(TTo))));

        return std::unique_ptr<TTo>(toptr);
    }

    #define $CASE_IMPL(...) { return __VA_ARGS__; }
    #define $case(T) [&](T) $CASE_IMPL
    #define $default(...) $case(auto) $CASE_IMPL(__VA_ARGS__)
}
