#pragma once

#include <array>
#include <cstddef>
#include <functional> // std::invoke
#include <string_view>
#include <utility>    // std::move, std::index_sequence_for

/// @file
/// @brief Compile-time string utilities and small metaprogramming helpers.
/// @details
/// Provides a fixed-size `ct_string<N>` that can be constructed at compile time,
/// concatenated, compared, and converted to `std::string_view`. Also includes:
/// - a consteval `to_ct_string<N>()` that renders an integer template arg to a `ct_string`,
/// - a user-defined literal `"_ct"` that turns a string literal into a `ct_string`,
/// - an overload set combiner `match{...}` for pattern-style visitation,
/// - a `foreach_indexed<Ts...>(fn)` that invokes `fn.template operator()<T, I>()` for each `T`.
///
/// @note Requires C++20 for class NTTPs used by the `"_ct"` literal.

namespace ctql {

    /**
     * @brief Fixed-size compile-time string.
     *
     * @tparam N Number of characters (not counting the trailing NUL).
     *
     * Holds `N+1` chars with a guaranteed `'@0'` terminator at `data[N]` when
     * constructed from a C string or produced by the provided helpers.
     */
    template <std::size_t N>
    struct ct_string {
        std::array<char, N + 1> data{}; ///< Storage including trailing NUL.

        /// @brief Default-construct with zero-initialized storage.
        constexpr ct_string() = default;

        /// @brief Construct from a C string literal of length `N`.
        constexpr ct_string(const char (&str)[N + 1]) {
            for (std::size_t i = 0; i < N + 1; ++i)
                data[i] = str[i];
        }

        /// @brief C-string pointer to the internal buffer (NUL-terminated).
        constexpr const char* c_str() const { return data.data(); }

        /// @brief Length excluding the terminator.
        constexpr std::size_t size() const { return N; }

        /// @brief Implicit view of the first `N` characters (excludes the terminator).
        constexpr operator std::string_view() const { return {data.data(), N}; }
    };

    /// @brief Deduction guide: `ct_string("abc")` becomes `ct_string<3>`.
    template <std::size_t N>
    ct_string(const char (&)[N]) -> ct_string<N - 1>;

    /**
     * @brief Concatenate two `ct_string`s.
     * @return A `ct_string<N1+N2>` containing `lhs` followed by `rhs`.
     * @complexity Linear in `N1 + N2`.
     */
    template <std::size_t N1, std::size_t N2>
    constexpr auto operator+(const ct_string<N1>& lhs, const ct_string<N2>& rhs) {
        ct_string<N1 + N2> result;
        for (std::size_t i = 0; i < N1; ++i)
            result.data[i] = lhs.data[i];
        for (std::size_t i = 0; i < N2; ++i)
            result.data[N1 + i] = rhs.data[i];
        result.data[N1 + N2] = '\0';
        return result;
    }

    /**
     * @brief Lexical equality for `ct_string`.
     * @complexity Linear in `min(N1, N2)`; short-circuits on size mismatch.
     */
    template <std::size_t N1, std::size_t N2>
    constexpr bool operator==(const ct_string<N1>& lhs, const ct_string<N2>& rhs) {
        if constexpr (N1 != N2)
            return false;
        else {
            for (std::size_t i = 0; i < N1; ++i) {
                if (lhs.data[i] != rhs.data[i])
                    return false;
            }
            return true;
        }
    }

    /**
     * @brief Count base-10 digits of a non-negative integer.
     * @note `consteval`: only usable in constant evaluation.
     */
    consteval std::size_t count_digits(std::size_t n) {
        std::size_t digits = 1;
        while (n /= 10)
            ++digits;
        return digits;
    }

    /**
     * @brief Render a non-type template argument `N` to a decimal `ct_string`.
     *
     * @tparam N The non-negative integer to render.
     * @return `ct_string<digits(N)>` with characters `'0'..'9'` and a trailing `'@0'`.
     */
    template <std::size_t N>
    consteval auto to_ct_string() {
        constexpr std::size_t digits = count_digits(N);
        ct_string<digits> result;
        std::size_t n = N;
        for (std::size_t i = 0; i < digits; ++i)
            result.data[digits - 1 - i] = char('0' + (n % 10)), n /= 10;
        result.data[digits] = '\0';
        return result;
    }

    /**
     * @brief User-defined literal: `"hello"_ct` -> `ct_string<5>{"hello"}`.
     *
     * @usage
     * @code
     * using namespace ctql;
     * constexpr auto s = "world"_ct;      // ct_string<5>
     * constexpr auto t = "hello"_ct + s;  // ct_string<10>
     * @endcode
     *
     * @note Requires C++20 class NTTPs.
     */
    template <ct_string str>
    constexpr auto operator""_ct() {
        return str;
    }

    /**
     * @brief Build an overload set from multiple callables (handy for `std::visit`).
     */
    template <typename... Lambda_t>
    struct match : Lambda_t... {
        using Lambda_t::operator()...;
        constexpr match(Lambda_t... lambda)
            : Lambda_t(std::move(lambda))... {}
    };

    /**
     * @brief Convenience macro to invoke a `match{...}` on a value.
     */
#define $match(arg, ...) std::invoke(ctql::match{__VA_ARGS__}, arg)

    /**
     * @brief Invoke `fn.template operator()<T, I>()` for each `T` in `Ts...`
     *        with its zero-based index `I`.
     *
     * @tparam Ts  The type sequence to enumerate.
     * @param fn   A callable whose `operator()` is a template taking `<T, I>`.
     *
     * @usage
     * @code
     * struct printer {
     *   template<class T, std::size_t I>
     *   void operator()() const {
     *     // use T and I
     *   }
     * };
     * ctql::foreach_indexed<int, double, char>(printer{});
     * @endcode
     */
    template <class... Ts, class Fn>
    constexpr void foreach_indexed(Fn&& fn) {
        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            (std::forward<Fn>(fn).template operator()<Ts, Is>(), ...);
        }(std::index_sequence_for<Ts...>{});
    }
}

