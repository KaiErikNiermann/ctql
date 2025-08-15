#define CTQL_ENABLE_DSL
#include <ctql.hpp>

using namespace ctql;

struct A {
    $size(10);
};
struct B {
    $size(20);
};
struct C {
    $size(5);
};
struct D {
    $size(15);
};
struct E {
    $size(25);
};
struct F {
    $size(20);
};

// ---- sorting ----

static_assert($type_eq($sort_types(A, B, C, D, E, F), $tuple_t(C, A, D, B, F, E)));

static_assert($type_eq($sort_types(A), $tuple_t(A)));

// ---- filtering ----

struct _N {
    $size(10);
};

static_assert($type_eq($filter_by(_N, $op("<="), A, B, C, D, E, F), $type_list(A, C)));

static_assert($type_eq($filter_by(_N, $op(">"), A, B, C, D, E, F), $type_list(B, D, E, F)));

static_assert($type_eq($filter_by(_N, $op("=="), A, B, C, D, E, F), $type_list(A)));

// ---- partitioning ----
static_assert($type_eq(
    $partition_by(_N, $op("<="), A, B, C, D, E, F),
    $type_list($type_list(A, C), $type_list(B, D, E, F))
));