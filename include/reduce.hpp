#pragma once

#include "htlist.hpp"
#include "predicates.hpp"
#include <type_traits>

/// @file
/// @brief Compile-time folds over `HasStaticSize` types.
/// @details
/// Folds a pack of types that expose a static size (via the `HasStaticSize` predicate)
/// using a user-supplied binary meta-op on `std::size_t`.
/// The public API is the `{reduce_sizes_t, reduce_sizes_v}` pair, plus `Sum{,_v}`.
/// Complexity is linear in the number of types.
///
/// ### Example
/// @code{.cpp}
/// struct A { static constexpr std::size_t size = 3; };
/// struct B { static constexpr std::size_t size = 5; };
/// static_assert(ctql::Sum_v<A,B> == 8);
/// @endcode
///
/// @note Works at compile time only; results are `std::integral_constant<size_t, ...>` or a value thereof.

namespace ctql {

    /// @brief Internal implementation details.
    /// @cond INTERNAL
    namespace detail {

        // fold_sizes<List, Op, Acc>:
        //   Recursively folds the HTList by applying Op to the accumulator and each head's size.
        template <typename List, template <std::size_t, std::size_t> class Op, std::size_t Acc>
        struct fold_sizes;

        // Base case: empty list -> accumulator.
        template <template <std::size_t, std::size_t> class Op, std::size_t Acc>
        struct fold_sizes<HTList<>, Op, Acc> : std::integral_constant<std::size_t, Acc> { };

        // Inductive case: peel head H (which satisfies HasStaticSize), push Op(Acc, H::size).
        template <
            HasStaticSize H,
            typename... Ts,
            template <std::size_t, std::size_t> class Op,
            std::size_t Acc>
        struct fold_sizes<HTList<H, Ts...>, Op, Acc>
            : fold_sizes<HTList<Ts...>, Op, Op<Acc, static_cast<std::size_t>(H::size)>::value> { };

        // Simple meta-ops on size_t.
        template <std::size_t A, std::size_t B>
        struct add_i : std::integral_constant<std::size_t, A + B> { };

        template <std::size_t A, std::size_t B>
        struct min_i : std::integral_constant<std::size_t, (A < B ? A : B)> { };

        template <std::size_t A, std::size_t B>
        struct max_i : std::integral_constant<std::size_t, (A < B ? B : A)> { };

    } // namespace detail
    /// @endcond

    /**
     * @brief Reduce a pack of `HasStaticSize` types by folding their `size` with a binary meta-op.
     *
     * @tparam Op  Binary meta-function class `Op<A,B>` that exposes `static constexpr size_t value`.
     *             It is applied left-to-right as `Op(... Op(Op(Init, size(T1)), size(T2)) ..., size(Tn))`.
     * @tparam Init Initial accumulator value.
     * @tparam Ts   Types satisfying `HasStaticSize` (must expose `static constexpr size` convertible to `size_t`).
     *
     * @returns Alias to `std::integral_constant<size_t, result>`.
     *
     * @par Complexity
     * Linear in `sizeof...(Ts)`.
     *
     * @see reduce_sizes_v, Sum, Sum_v
     */
    template <template <std::size_t, std::size_t> class Op, std::size_t Init, typename... Ts>
    using reduce_sizes_t = detail::fold_sizes<detail::HTList<Ts...>, Op, Init>;

    /**
     * @brief Convenience value for @ref reduce_sizes_t.
     * @see reduce_sizes_t
     */
    template <template <std::size_t, std::size_t> class Op, std::size_t Init, typename... Ts>
    inline constexpr std::size_t reduce_sizes_v = reduce_sizes_t<Op, Init, Ts...>::value;

    /**
     * @brief Sum of `Ts::size` for a pack of `HasStaticSize` types.
     *
     * @tparam Ts Types satisfying `HasStaticSize`.
     * @returns Alias to `std::integral_constant<size_t, sum>`.
     *
     * @note Empty pack yields `0`.
     */
    template <typename... Ts>
    using Sum = reduce_sizes_t<detail::add_i, 0, Ts...>;

    /// @brief Convenience value for @ref Sum.
    template <typename... Ts>
    inline constexpr std::size_t Sum_v = Sum<Ts...>::value;

} // namespace ctql
