#pragma once 

#include "htlist.hpp"
#include "predicates.hpp"
#include <type_traits>

namespace ctql {
        namespace detail {
        template <typename Pivot, template <typename, typename> class Rel, typename List>
        struct partition_by_impl;

        template <typename Pivot, template <typename, typename> class Rel>
        struct partition_by_impl<Pivot, Rel, HTList<>> {
            using pass = HTList<>; // elements where Rel<Pivot, Elem> is true
            using fail = HTList<>; // the rest
        };

        template <
            typename Pivot,
            template <typename, typename> class Rel,
            typename H,
            typename... Ts>
        struct partition_by_impl<Pivot, Rel, HTList<H, Ts...>> {
        private:
            using rest = partition_by_impl<Pivot, Rel, HTList<Ts...>>;

        public:
            using pass = std::conditional_t<
                Rel<Pivot, H>::value,
                typename append<HTList<H>, typename rest::pass>::type,
                typename rest::pass>;
            using fail = std::conditional_t<
                Rel<Pivot, H>::value,
                typename rest::fail,
                typename append<HTList<H>, typename rest::fail>::type>;
        };
    } // namespace detail

    template <typename Pivot, template <typename, typename> class Rel, typename... Ts>
    struct partition_by {
        using impl   = detail::partition_by_impl<Pivot, Rel, detail::HTList<Ts...>>;
        using pass   = typename impl::pass; // “kept” / left bucket
        using fail   = typename impl::fail; // “rejected” / right bucket
        using concat = typename detail::HTList<pass, fail>;
    };

    template <typename Pivot, template <typename, typename> class Rel, typename... Ts>
    using filter_by = typename partition_by<Pivot, Rel, Ts...>::pass;

    template <typename Pivot, template <typename, typename> class Rel, typename... Ts>
    using reject_if_by = typename partition_by<Pivot, Rel, Ts...>::fail;

    template <
        typename Pivot,
        template <typename, typename> class Rel,
        template <typename> class KeyOf = Size,
        typename... Ts>
    using partition_by_key = partition_by<Pivot, Rel, KeyOf<Ts>...>;
}