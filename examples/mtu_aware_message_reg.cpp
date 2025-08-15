#include <variant>
#include <tuple>
#include <type_traits>
#include <ctql.hpp>

using namespace ctql;

template <typename P, typename E>
struct leq_size {
    static constexpr bool value = (E::size <= P::size);
};
template <typename P, typename E>
struct gt_size {
    static constexpr bool value = (E::size > P::size);
};

struct MsgLogin { };
struct MsgPing { };
struct MsgChunk { };
struct MsgTelemetry { };

template <class>
struct WireBytes;
template <>
struct WireBytes<MsgLogin> : std::integral_constant<std::size_t, 48> { };
template <>
struct WireBytes<MsgPing> : std::integral_constant<std::size_t, 16> { };
template <>
struct WireBytes<MsgChunk> : std::integral_constant<std::size_t, 4096> { };
template <>
struct WireBytes<MsgTelemetry> : std::integral_constant<std::size_t, 256> { };

template <class T>
struct WireSizeOf {
    using type = T;
    $size(WireBytes<T>::value);
};

// Pivot type for MTU threshold
struct MTU1200 {
    $size(1200);
};

// Build the wrapper set once
using MsgSet
    = $type_list(WireSizeOf<MsgLogin>, WireSizeOf<MsgPing>, WireSizeOf<MsgChunk>, WireSizeOf<MsgTelemetry>);

// Partition in one pass (<= MTU vs > MTU)
using P = $partition_by_key( /* key = */ WireSizeOf, MTU1200, leq_size, MsgLogin, MsgPing, MsgChunk, MsgTelemetry);

using FitsMtuSet = typename P::pass; // HTList<WireSizeOf<...>>
using TooBigSet  = typename P::fail;

// Turn buckets into variants of original types
using FitsMtu   = $to_variant(FitsMtuSet); // std::variant<MsgLogin, MsgPing, MsgTelemetry>
using NeedsFrag = $to_variant(TooBigSet); // std::variant<MsgChunk>

// Quick summaries
static_assert($sum_sizes(WireSizeOf<MsgLogin>, WireSizeOf<MsgPing>) == 64);
static_assert($sum_sizes(WireSizeOf<MsgChunk>) == 4096);

// You can also sort the MTU-fitting set by size for “send small first” policy:
using FitsMtuSortedWrappers
    = ctql::TypeSort<ctql::Order::Asc, WireSizeOf, MsgLogin, MsgPing, MsgChunk, MsgTelemetry>;

using FitsMtuSortedTuple
    = $to_tuple(FitsMtuSortedWrappers); // std::tuple<MsgPing, MsgLogin, MsgTelemetry>
