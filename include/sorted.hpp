#pragma once

#include "partition.hpp"
#include "predicates.hpp"
#include "htlist.hpp"

/// @file
/// @brief Compile-time quicksort over a heterogenous typelist.
/// @details
/// Sorts a `detail::HTList<...>` whose elements satisfy `HasStaticSize`
/// (i.e., each exposes a `static constexpr size` convertible to `std::size_t`).
/// Order is controlled by `Order::{Asc,Desc}` and comparisons are routed through
/// `op_type<"...">` predicates like `"<"_ct`, `">="_ct`, etc.
///
/// **Example**
///
/// ```cpp
/// struct A { static constexpr std::size_t size = 3; };
/// struct B { static constexpr std::size_t size = 1; };
/// struct C { static constexpr std::size_t size = 2; };
///
/// using L = ctql::detail::HTList<A, B, C>;
/// using Sorted = ctql::sort_list<ctql::Order::Asc, L>::type; // ==> HTList<B, C, A>
/// ```
///
/// **Key mapping with @ref TypeSort**
///
/// ```cpp
/// // If Size<T> is a "key" type that itself satisfies HasStaticSize (e.g., Size<T>::size = T::size),
/// // TypeSort produces a list of the keys (not the original Ts):
/// using Out = ctql::TypeSort<ctql::Order::Asc, ctql::Size, A, B, C>; // HTList<Size<B>, Size<C>, Size<A>>
/// ```
///
/// @note This is a quicksort-style split on the first element; it is not stable.
/// @complexity Expected O(n log n) template instantiations; worst-case O(n^2).

namespace ctql {

    /// @brief Sort direction.
    enum class Order { Asc, Desc };

    /**
     * @brief Primary template for sorting an `HTList`.
     * @tparam Ord  Sort order (ascending or descending).
     * @tparam List A `detail::HTList<...>` whose elements satisfy `HasStaticSize`.
     *
     * @details The resulting type is exposed as `sort_list<...>::type`, an `HTList<...>`
     * in the requested order. Internally uses a partition + recursion scheme.
     */
    template <Order Ord, typename List>
    struct sort_list;

    /// @brief Empty list stays empty.
    template <Order Ord>
    struct sort_list<Ord, detail::HTList<>> {
        using type = detail::HTList<>;
    };

    /**
     * @brief Quicksort step using the head `T0` as the pivot.
     * @tparam Ord    Sort order.
     * @tparam T0     Pivot; must satisfy `HasStaticSize`.
     * @tparam TRest  Remaining elements; each must satisfy `HasStaticSize`.
     *
     * @details
     * - Left side collects elements strictly less than (or strictly greater than, for Desc) the pivot.
     * - Right side collects the rest (greater-equal / less-equal).
     * - Recursively sorts both sides and concatenates: `LeftSorted ++ [T0] ++ RightSorted`.
     */
    template <Order Ord, HasStaticSize T0, HasStaticSize... TRest>
    struct sort_list<Ord, detail::HTList<T0, TRest...>> {
    private:
        // Choose relational operators based on order:
        //   Asc:  Left = "<",  Right = ">="
        //   Desc: Left = ">",  Right = "<="
        using LeftRel  = std::conditional_t<Ord == Order::Asc,  op_type<"<"_ct>,  op_type<">"_ct>>;
        using RightRel = std::conditional_t<Ord == Order::Asc,  op_type<">="_ct>, op_type<"<="_ct>>;

        // Partition the tail TRest... with respect to pivot T0.
        // P::pass  => elements on the Left side (strict compare vs pivot)
        // Not-pass => elements on the Right side (>= or <= vs pivot)
        using P = partition_by<T0, LeftRel::template pred, TRest...>;
        using LeftSorted = typename sort_list<Ord, typename P::pass>::type;

        using RightUnsorted = filter_by<T0, RightRel::template pred, TRest...>;
        using RightSorted   = typename sort_list<Ord, RightUnsorted>::type;

    public:
        // Concatenate: LeftSorted ++ [T0] ++ RightSorted
        using type = typename detail::append<
            typename detail::append<LeftSorted, detail::HTList<T0>>::type,
            RightSorted
        >::type;
    };

    /**
     * @brief Convenience alias to sort a pack via a key-mapping.
     * @tparam Ord    Sort order (default: @c Order::Asc).
     * @tparam KeyOf  Unary metafunction class `KeyOf<T>` producing a type that satisfies `HasStaticSize`.
     * @tparam Ts     Input type pack.
     *
     * @returns `detail::HTList<KeyOf<Ts>...>` sorted by each key type's `size`.
     *
     * **Example**
     *
     * ```cpp
     * // Suppose Size<T> is a key-type with Size<T>::size == T::size.
     * using SortedKeys = ctql::TypeSort<ctql::Order::Asc, ctql::Size, A, B, C>;
     * // => HTList<Size<B>, Size<C>, Size[A]>
     * ```
     *
     * @note This alias sorts the *key types* `KeyOf<Ts>`, not the original `Ts`.
     *       If you need the original `Ts` in sorted order, carry them alongside
     *       the keys in your list representation and project them back after sorting.
     */
    template <Order Ord = Order::Asc, template <typename> class KeyOf = Size, typename... Ts>
    using TypeSort = typename sort_list<Ord, detail::HTList<KeyOf<Ts>...>>::type;

} // namespace ctql
