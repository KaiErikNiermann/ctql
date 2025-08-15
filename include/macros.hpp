// ctql/macros.hpp
#pragma once
// Public macro layer for ctql. Define CTQL_NO_MACROS to opt out entirely.
// Define CTQL_ENABLE_DSL to also export `$...` shorthands (non-standard identifiers).

#ifndef CTQL_NO_MACROS

// ---- Core CTQL_* macros (portable, recommended) ----
#define CTQL_TYPE_LIST(...)            ::ctql::detail::HTList<__VA_ARGS__>
#define CTQL_APPEND(ListL, ListR)      typename ::ctql::detail::append<ListL, ListR>::type

// Extractors
#define CTQL_TO_TUPLE(List)            typename ::ctql::to_tuple<List>::type
#define CTQL_TO_VARIANT(List)          typename ::ctql::to_variant<List>::type
#define CTQL_TUPLE_T(...)              std::tuple<__VA_ARGS__>

// Partition API
#define CTQL_PARTITION_BY(Pivot, Rel, ...)   ::ctql::partition_by<Pivot, Rel, __VA_ARGS__>
#define CTQL_PARTITION_PASS(Pivot, Rel, ...) typename CTQL_PARTITION_BY(Pivot, Rel, __VA_ARGS__)::pass
#define CTQL_PARTITION_FAIL(Pivot, Rel, ...) typename CTQL_PARTITION_BY(Pivot, Rel, __VA_ARGS__)::fail
#define CTQL_PARTITION_CONCAT(Pivot, Rel, ...) typename CTQL_PARTITION_BY(Pivot, Rel, __VA_ARGS__)::concat
#define CTQL_PARTITION_BY_KEY(Pivot, Rel, KeyOf, ...) \
    ::ctql::partition_by_key<Pivot, Rel, KeyOf, __VA_ARGS__>

// Sorting (KeyOf<T> must provide: using type = T; static constexpr size)
#define CTQL_SORT_TYPES(...)              ::ctql::TypeSort<::ctql::Order::Asc, Size, __VA_ARGS__>
#define CTQL_SORT_TYPES_ASC(KeyOf, ...)   ::ctql::TypeSort<::ctql::Order::Asc,  KeyOf, __VA_ARGS__>
#define CTQL_SORT_TYPES_DESC(KeyOf, ...)  ::ctql::TypeSort<::ctql::Order::Desc, KeyOf, __VA_ARGS__>

// Reducers over `.size`
#define CTQL_REDUCE_SIZES(Op, Init, ...)  (::ctql::reduce_sizes_v<Op, Init, __VA_ARGS__>)
#define CTQL_SUM_SIZES(...)               (::ctql::Sum_v<__VA_ARGS__>)

// Matching 
#define CTQL_MATCH(arg, ...)               std::invoke(ctql::match {__VA_ARGS__}, arg)

// Operation
#define CTQL_OP(str)                      ctql::op_type<str##_ct>::template pred

// Predicates 
#define CTQL_SIZE_OF(...)              ctql::SizeOf_t<__VA_ARGS__>
#define CTQL_ALIGN_OF(...)            ctql::AlignOf_t<__VA_ARGS__>
#define CTQL_APPLY(S, ...)         ctql::Apply<S, __VA_ARGS__>

// Convenience for defining a "size" in wrappers or pivots
#define CTQL_SIZE(n) static constexpr std::size_t size = (n);

#  ifdef CTQL_ENABLE_DSL
    // Lists / transforms
#   define $type_list(...)          CTQL_TYPE_LIST(__VA_ARGS__)
#   define $to_tuple(List)          CTQL_TO_TUPLE(List)
#   define $to_variant(List)        CTQL_TO_VARIANT(List)
#   define $tuple_t(...)            CTQL_TUPLE_T(__VA_ARGS__)

    // Partition helpers
#   define $partition_by(Pivot, Rel, ...)   CTQL_PARTITION_CONCAT(Pivot, Rel, __VA_ARGS__)
#   define $filter_by(Pivot, Rel, ...)      CTQL_PARTITION_PASS(Pivot, Rel, __VA_ARGS__)
#   define $reject_if_by(Pivot, Rel, ...)   CTQL_PARTITION_FAIL(Pivot, Rel, __VA_ARGS__)

    // Sorting short-hands (defaults to ascending)
#   define $sort_types(...)             CTQL_TO_TUPLE(CTQL_SORT_TYPES(__VA_ARGS__))
#   define $sort_types_by(KeyOf, ...)   CTQL_TO_TUPLE(CTQL_SORT_TYPES_ASC(KeyOf, __VA_ARGS__))
#   define $sort_types_desc(KeyOf, ...) CTQL_TO_TUPLE(CTQL_SORT_TYPES_DESC(KeyOf, __VA_ARGS__))

    // Reducers
#   define $reduce_sizes(...)         CTQL_REDUCE_SIZES(__VA_ARGS__)  // (Op, Init, Ts...)
#   define $sum_sizes(...)            CTQL_SUM_SIZES(__VA_ARGS__)     // (Ts...)
#   define $size(n)                   CTQL_SIZE(n)

    // Matching
#   define $match(arg, ...)           CTQL_MATCH(arg, __VA_ARGS__)

    // Operation
#   define $op(str)                   CTQL_OP(str)

    // Predicates
#   define $size_of(...)              CTQL_SIZE_OF(__VA_ARGS__)
#   define $align_of(...)             CTQL_ALIGN_OF(__VA_ARGS__)
#   define $apply(S, ...)             CTQL_APPLY(S, __VA_ARGS__)

#  endif // CTQL_ENABLE_DSL

#endif // CTQL_NO_MACROS
