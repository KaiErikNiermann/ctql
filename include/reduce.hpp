#pragma once 

#include "htlist.hpp"
#include "predicates.hpp"
#include <type_traits>

namespace ctql {
        /// @brief compile-time fold
    namespace detail {
        template <typename List, template <std::size_t, std::size_t> class Op, std::size_t Acc>
        struct fold_sizes;

        template <template <std::size_t, std::size_t> class Op, std::size_t Acc>
        struct fold_sizes<HTList<>, Op, Acc> : std::integral_constant<std::size_t, Acc> { };

        template <
            HasStaticSize H,
            typename... Ts,
            template <std::size_t, std::size_t> class Op,
            std::size_t Acc>
        struct fold_sizes<HTList<H, Ts...>, Op, Acc>
            : fold_sizes<HTList<Ts...>, Op, Op<Acc, static_cast<std::size_t>(H::size)>::value> { };

        // ---------- size operations ----------
        template <std::size_t A, std::size_t B>
        struct add_i : std::integral_constant<std::size_t, A + B> { };

        template <std::size_t A, std::size_t B>
        struct min_i : std::integral_constant<std::size_t, (A < B ? A : B)> { };

        template <std::size_t A, std::size_t B>
        struct max_i : std::integral_constant<std::size_t, (A < B ? B : A)> { };
        // -------------------------------------
    } // namespace detail

    template <template <std::size_t, std::size_t> class Op, std::size_t Init, typename... Ts>
    using reduce_sizes_t = detail::fold_sizes<detail::HTList<Ts...>, Op, Init>;

    template <template <std::size_t, std::size_t> class Op, std::size_t Init, typename... Ts>
    inline constexpr std::size_t reduce_sizes_v = reduce_sizes_t<Op, Init, Ts...>::value;

    template <typename... Ts>
    using Sum = reduce_sizes_t<detail::add_i, 0, Ts...>;

    template <typename... Ts>
    inline constexpr std::size_t Sum_v = Sum<Ts...>::value;
}