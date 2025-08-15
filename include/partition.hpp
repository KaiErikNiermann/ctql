#pragma once

#include "htlist.hpp"
#include "predicates.hpp"
#include <type_traits>

/// @file
/// @brief Compile-time partitioning of a typelist based on a binary relation.
/// @details
/// Splits an `HTList<Ts...>` into two lists: those that satisfy a relation
/// `Rel<Pivot, T>::value` (the **pass** bucket) and those that don't (the
/// **fail** bucket). Aliases `filter_by` and `reject_if_by` expose the two
/// buckets directly, and `partition_by_key` lets you partition on derived
/// key types (e.g., `Size<T>`).
///
/// **Example**
///
/// ```cpp
/// // Given Rel that compares Elem vs Pivot by size (see PredBy/op_tag in predicates.hpp):
/// using L = ctql::detail::HTList<A, B, C>;
/// using P = ctql::partition_by<A, LessRel::template pred, B, C>;
/// using Left = P::pass; // elements X with LessRel::pred<A, X>::value == true
/// using Right = P::fail;
/// ```
///
/// @note `Rel<Pivot, T>` must define a `static constexpr bool value`.

namespace ctql {

    /// @cond INTERNAL
    namespace detail {

        // partition_by_impl<Pivot, Rel, HTList<...>>
        // Produces two buckets:
        //   - pass: elements where Rel<Pivot, Elem>::value is true
        //   - fail: the remaining elements
        template <typename Pivot, template <typename, typename> class Rel, typename List>
        struct partition_by_impl;

        // Empty list -> both buckets empty.
        template <typename Pivot, template <typename, typename> class Rel>
        struct partition_by_impl<Pivot, Rel, HTList<>> {
            using pass = HTList<>;
            using fail = HTList<>;
        };

        // Step: peel head H; recurse on tail Ts...
        template <typename Pivot,
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
    /// @endcond

    /**
     * @brief Partition a pack `Ts...` into @c pass / @c fail buckets.
     * @tparam Pivot The pivot type used by the relation.
     * @tparam Rel   A binary template `Rel<L,R>` exposing `static constexpr bool value`.
     * @tparam Ts    Elements to partition.
     *
     * @details Builds two typelists:
     * - @c pass: elements @c T where `Rel<Pivot, T>::value` is @c true
     * - @c fail: the remaining elements
     *
     * @remarks The concatenation order of results is stable relative to the
     *          original order (within each bucket).
     */
    template <typename Pivot, template <typename, typename> class Rel, typename... Ts>
    struct partition_by {
        using impl   = detail::partition_by_impl<Pivot, Rel, detail::HTList<Ts...>>;
        using pass   = typename impl::pass; // kept / left bucket
        using fail   = typename impl::fail; // rejected / right bucket
        using concat = typename detail::HTList<pass, fail>; // pair as a two-element typelist
    };

    /**
     * @brief Keep elements @c T where `Rel<Pivot, T>::value` is @c true.
     * @tparam Pivot Pivot type.
     * @tparam Rel   Relation template.
     * @tparam Ts    Elements to filter.
     */
    template <typename Pivot, template <typename, typename> class Rel, typename... Ts>
    using filter_by = typename partition_by<Pivot, Rel, Ts...>::pass;

    /**
     * @brief Reject elements @c T where `Rel<Pivot, T>::value` is @c true.
     * @tparam Pivot Pivot type.
     * @tparam Rel   Relation template.
     * @tparam Ts    Elements to test.
     */
    template <typename Pivot, template <typename, typename> class Rel, typename... Ts>
    using reject_if_by = typename partition_by<Pivot, Rel, Ts...>::fail;

    /**
     * @brief Partition after mapping each element through a key wrapper.
     * @tparam Pivot Pivot type (compared against key types).
     * @tparam Rel   Relation template over key types.
     * @tparam KeyOf Unary template mapping @c T -> @c KeyOf<T> (defaults to @ref Size).
     * @tparam Ts    Elements to partition.
     *
     * @details Applies @c KeyOf to every @c T, then partitions on the resulting
     * key types: `partition_by<Pivot, Rel, KeyOf<Ts>...>`.
     *
     * **Example**
     *
     * ```cpp
     * // Use Size<T> keys (which expose ::size) to partition by size relative to Pivot:
     * using P = ctql::partition_by_key<Pivot, LessRel::template pred, ctql::Size, A, B, C>;
     * ```
     */
    template <typename Pivot,
              template <typename, typename> class Rel,
              template <typename> class KeyOf = Size,
              typename... Ts>
    using partition_by_key = partition_by<Pivot, Rel, KeyOf<Ts>...>;

} // namespace ctql
