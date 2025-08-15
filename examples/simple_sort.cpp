#define CTQL_ENABLE_DSL
#include <tuple>
#include <type_traits>
#include <ctql.hpp>
using namespace ctql;

struct A { double x[4]; }; // 32B
struct B { int    y;    }; // 4B
struct C { char   z[9]; }; // 9B

// Sort by sizeof ascending, then materialize into a tuple:
using SortedWrappers = TypeSort<Order::Asc, SizeOf, A, B, C>;   // HTList<SizeOf<B>, SizeOf<C>, SizeOf<A>>
using SortedTuple    = typename to_tuple<SortedWrappers>::type; // std::tuple<B, C, A>

// Macro syntax 
static_assert($type_eq(
    $sort_types_by(SizeOf, A, B, C), 
    $tuple_t(B, C, A) 
));

static_assert(std::is_same_v<SortedTuple, std::tuple<B, C, A>>);

// Sort by alignof descending:
using AlignSorted = TypeSort<Order::Desc, AlignOf, A, B, C>;
using AlignTuple  = typename to_tuple<AlignSorted>::type; // order depends on alignof(...)