#pragma once
#include "htlist.hpp"
#include <concepts>
#include <cstddef>
#include <functional>
#include <ct_string.inl>
#include <match.hpp>

/// @file
/// @brief Size/align key wrappers, comparison predicates, and operator tag mapping.
/// @details
/// - `HasStaticSize` concept gates types that expose `static constexpr std::size_t size`.
/// - `Size<T>`, `SizeOf<T>`, `AlignOf<T>` present a unified `::type` and `::size`.
/// - `cmp`, `PredBy`, `op_tag`, and `ops` build size-based predicates.
/// - `op_type<"...">` maps a compile-time string token (e.g. `"<"_ct`) to an operator tag
///   whose `template pred<L,R>` can be used in algorithms (e.g. partition/sort).
/// - Helpers `Size_t`, `SizeOf_t`, `AlignOf_t`, and `Apply<S,...>` create keyed typelists.
///
/// ### Example
///
/// @code{.cpp}
/// struct A { static constexpr std::size_t size = 3; };
/// struct B { static constexpr std::size_t size = 5; };
///
/// using LT = ctql::op_type<"<"_ct>;            // operator tag for less-than
/// static_assert(LT::template pred<A,B>::value); // 3 < 5
///
/// using Keys = ctql::Size_t<A,B>;              // HTList<Size<A>, Size<B>>
/// @endcode

namespace ctql {

    /**
     * @brief Concept for types that expose a static size.
     * @tparam T The type under test.
     * @remarks Requires `T::size` convertible to `std::size_t`.
     */
    template <class T>
    concept HasStaticSize = requires {
        { T::size } -> std::convertible_to<std::size_t>;
    };

    /**
     * @brief Key wrapper that forwards `T::size`.
     * @tparam T Any type that has `static constexpr std::size_t size`.
     */
    template <typename T>
    struct Size {
        using type                        = T;
        static constexpr std::size_t size = T::size;
    };

    /**
     * @brief Key wrapper that uses `sizeof(T)`.
     * @tparam T Any complete type.
     */
    template <typename T>
    struct SizeOf {
        using type                        = T;
        static constexpr std::size_t size = sizeof(T);
    };

    /**
     * @brief Key wrapper that uses `alignof(T)`.
     * @tparam T Any complete type.
     */
    template <typename T>
    struct AlignOf {
        using type                        = T;
        static constexpr std::size_t size = alignof(T);
    };

    /**
     * @brief Compare two `HasStaticSize` types using a standard comparator.
     * @tparam Cmp A callable like `std::less<>`, `std::greater_equal<>`, etc.
     * @tparam A   Left operand (provides `A::size`).
     * @tparam B   Right operand (provides `B::size`).
     */
    template <typename Cmp, HasStaticSize A, HasStaticSize B>
    struct cmp {
        static constexpr bool value
            = Cmp{}(static_cast<std::size_t>(A::size), static_cast<std::size_t>(B::size));
    };

    /**
     * @brief Build a pivot-relative predicate from a comparator.
     * @tparam Cmp Comparator applied to `(Elem::size, Pivot::size)`.
     *
     * @details `PredBy<Cmp>::t<Pivot, Elem>::value` is `cmp<Cmp, Elem, Pivot>::value`.
     * Useful in partitioning where you compare each element to a fixed pivot.
     */
    template <typename Cmp>
    struct PredBy {
        template <HasStaticSize Pivot, HasStaticSize Elem>
        struct t {
            static constexpr bool value = cmp<Cmp, Elem, Pivot>::value;
        };
    };

    /**
     * @brief Operator tag that exposes a `pred<L,R>` alias for use in templates.
     * @tparam Cmp Comparator used inside the predicate.
     */
    template <typename Cmp>
    struct op_tag {
        template <typename L, typename R>
        using pred = typename PredBy<Cmp>::template t<L, R>;
    };

    /**
     * @brief Standard operator tags on sizes.
     *
     * - `leq`: `<=`
     * - `geq`: `>=`
     * - `lt` : `<`
     * - `gt` : `>`
     * - `eq` : `==`
     * - `neq`: `!=`
     */
    struct ops {
        using leq = op_tag<std::less_equal<>>;
        using geq = op_tag<std::greater_equal<>>;
        using lt  = op_tag<std::less<>>;
        using gt  = op_tag<std::greater<>>;
        using eq  = op_tag<std::equal_to<>>;
        using neq = op_tag<std::not_equal_to<>>;
    };

    /**
     * @brief Map a compile-time operator token to an operator tag.
     * @tparam str Token such as `"<"_ct`, `">="_ct`, `"=="_ct`, `"!="_ct`.
     *
     * @returns One of the tags in @ref ops. If unknown, yields `void`.
     *
     * **Example**
     *
     * @code{.cpp}
     * using Tag = ctql::op_type<">="_ct>;
     * static_assert(Tag::template pred<A,B>::value == (A::size >= B::size));
     * @endcode     
     */
    template <ct_string str>
    using op_type = ctql::match_t<
        str,
        case_<"<="_ct, ops::leq>,
        case_<"<"_ct,  ops::lt>,
        case_<">="_ct, ops::geq>,
        case_<">"_ct,  ops::gt>,
        case_<"=="_ct, ops::eq>,
        case_<"!="_ct, ops::neq>,
        default_<void>
    >;

    /**
     * @brief Build a typelist of `Size<Ts>...`.
     * @tparam Ts Input types.
     */
    template <typename... Ts>
    using Size_t = detail::HTList<Size<Ts>...>;

    /**
     * @brief Build a typelist of `SizeOf<Ts>...`.
     * @tparam Ts Input types.
     */
    template <typename... Ts>
    using SizeOf_t = detail::HTList<SizeOf<Ts>...>;

    /**
     * @brief Build a typelist of `AlignOf<Ts>...`.
     * @tparam Ts Input types.
     */
    template <typename... Ts>
    using AlignOf_t = detail::HTList<AlignOf<Ts>...>;

    /**
     * @brief Apply a unary key wrapper over a pack into an `HTList`.
     * @tparam S  Unary template taking `T` (e.g., `Size`, `SizeOf`, `AlignOf`).
     * @tparam Ts Input types.
     */
    template <template <typename> class S, typename... Ts>
    using Apply = detail::HTList<S<Ts>...>;

} // namespace ctql
