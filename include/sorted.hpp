#include "partition.hpp"
#include "predicates.hpp"
#include "htlist.hpp"

namespace ctql {
       enum class Order { Asc, Desc };

    template <Order Ord, typename List>
    struct sort_list;

    template <Order Ord>
    struct sort_list<Ord, detail::HTList<>> {
        using type = detail::HTList<>;
    };

    template <Order Ord, HasStaticSize T0, HasStaticSize... TRest>
    struct sort_list<Ord, detail::HTList<T0, TRest...>> {
    private:
        using LeftRel = std::conditional_t<
            Ord == Order::Asc,
            op_type<"<"_ct>, // Elem < Pivot
            op_type<">"_ct>>; // Elem > Pivot
        using RightRel = std::conditional_t<
            Ord == Order::Asc,
            op_type<">="_ct>, // Elem >= Pivot
            op_type<"<="_ct>>; // Elem <= Pivot

        using P
            = partition_by<T0, LeftRel::template pred, TRest...>; // pass = Left, fail = not-left
        using LeftSorted = typename sort_list<Ord, typename P::pass>::type;

        using RightUnsorted = filter_by<T0, RightRel::template pred, TRest...>;
        using RightSorted   = typename sort_list<Ord, RightUnsorted>::type;

    public:
        using type = typename detail::append<
            typename detail::append<LeftSorted, detail::HTList<T0>>::type,
            RightSorted>::type;
    };

    template <Order Ord = Order::Asc, template <typename> class KeyOf = Size, typename... Ts>
    using TypeSort = typename sort_list<Ord, detail::HTList<KeyOf<Ts>...>>::type;
}