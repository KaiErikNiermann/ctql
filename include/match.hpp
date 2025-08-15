#pragma once
// ctql/match.hpp

#include <type_traits>

/// @file
/// @brief Compile-time key -> type matcher (first-match wins).
/// @details
/// Provides a tiny pattern-matching facility that selects a type based on a
/// compile-time key. You declare alternatives with `case_<K, T>` and an
/// optional `default_<T>`. The alias `match_t<Key, Alts...>` yields the
/// selected `T`.
///
/// ### Example
///
/// @code{.cpp}
/// using A = struct A_;
/// using B = struct B_;
/// using D = struct D_;
///
/// // Picks B because Key == 'b' matches the second case.
/// using R = ctql::match_t<'b',
///                 ctql::case_<'a', A>,
///                 ctql::case_<'b', B>,
///                 ctql::default_<D>>;
/// static_assert(std::is_same_v<R, B>);
///
/// // If no case matches and no default_ is provided, result is void:
/// using R2 = ctql::match_t<42, ctql::case_<0, A>>;
/// static_assert(std::is_void_v<R2>);
/// @endcode
///
/// @note Keys are compared with `==` at compile time. This works for integral,
///       enum, and other literal NTTPs that support `constexpr` equality.

namespace ctql {

    /**
     * @brief A single match alternative.
     * @tparam K   Compile-time key for this case (auto NTTP).
     * @tparam T   Type produced when `Key == K`.
     */
    template <auto K, class T>
    struct case_ {
        static constexpr auto key = K;
        using type                = T;
    };

    /**
     * @brief Fallback alternative if nothing matched earlier.
     * @tparam T Type produced if no prior case_ matched.
     */
    template <class T>
    struct default_ {
        using type = T;
    };

    /// @cond INTERNAL
    namespace detail {
        // -------- matcher core (first-match wins; default if no match) --------
        template <auto Key, class Default, bool Matched, class... Alts>
        struct match_impl;

        // End of list -> yield accumulated Default.
        template <auto Key, class Default, bool Matched>
        struct match_impl<Key, Default, Matched> {
            using type = Default;
        };

        // case_ step: if not matched yet and Key == K, lock in T; otherwise keep going.
        template <auto Key, class Default, bool Matched, auto K, class T, class... Rest>
        struct match_impl<Key, Default, Matched, case_<K, T>, Rest...>
            : match_impl<
                  Key,
                  std::conditional_t<!Matched && (Key == K), T, Default>,
                  (Matched || (Key == K)),
                  Rest...> { };

        // default_ step: if not matched yet, update fallback; otherwise ignore.
        template <auto Key, class Default, bool Matched, class T, class... Rest>
        struct match_impl<Key, Default, Matched, default_<T>, Rest...>
            : match_impl<Key, std::conditional_t<Matched, Default, T>, Matched, Rest...> { };

        // Public interface wrapper.
        template <auto Key, class... Alts>
        struct match {
            using type = typename match_impl<Key, void, false, Alts...>::type;
        };
    } // namespace detail
    /// @endcond

    /**
     * @brief Select a type based on a compile-time key.
     * @tparam Key   The key to match against `case_<K, T>` alternatives.
     * @tparam Alts  Zero or more `case_<K, T>` and at most one `default_<T>`.
     *
     * @returns The chosen `T` as a type alias. If no case matches and no
     *          `default_` is provided, the result is `void`.
     *
     * @note
     * - First-match wins: the earliest `case_` with `Key == K` is chosen.
     * - `default_<T>` sets the fallback if no earlier case matched. A later
     *   match still overrides the default (since it is “first-match wins”).
     */
    template <auto Key, class... Alts>
    using match_t = typename detail::match<Key, Alts...>::type;

} // namespace ctql
