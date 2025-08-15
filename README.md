# ctql — Compile-Time Query Language (for types)

Header-only C++23 toolkit to **filter/partition/sort/reduce *types*** using user-defined keys (like `sizeof(T)`, `alignof(T)`, or your own compile-time metric). Also includes tiny `ct_string` helpers and nicer concept/predicate wrappers.

* **Single header:** `#include <ctql/ctql.hpp>`
* **Standard:** C++23
* **Compiler:** Clang++ 18 (tested)

---

## Why ctql?

* Treat type packs like a queryable dataset: **partition**, **sort**, **reduce** at compile time.
* Bring your own key: `KeyOf<T> → size` (e.g., `sizeof`, `alignof`, wire size, priority).
* Zero runtime cost. Great for metaprogramming, registries, ECS layouts, SBO/MTU policies.

---

## Install

Just drop the `ctql/` folder in your include path.

```bash
clang++-18 -std=c++23 -Wall -Wextra -pedantic demo.cpp -Ipath/to/ctql
```

```cpp
#include <ctql/ctql.hpp>
using namespace ctql;
```

---

## Quick tour

We’ll use the **KeyOf** wrapper pattern: turn a concrete type `T` into a meta “row” with a `static constexpr size` and `using type = T;`.

```cpp
// Key extractors (you can define your own)
template <class T>
struct SizeOf { using type = T; static constexpr std::size_t size = sizeof(T); };

template <class T>
struct AlignOf { using type = T; static constexpr std::size_t size = alignof(T); };

// A simple binary relation: keep Elem if Elem.size <= Pivot.size
template <class Pivot, class Elem>
struct leq_size { static constexpr bool value = (Elem::size <= Pivot::size); };
```

### 1) Sort types by a key (ascending/descending)

```cpp
#include <tuple>
#include <type_traits>
#include <ctql/ctql.hpp>
using namespace ctql;

struct A { double x[4]; }; // 32B
struct B { int    y;    }; // 4B
struct C { char   z[9]; }; // 9B

// Sort by sizeof ascending, then materialize into a tuple:
using SortedWrappers = TypeSort<Order::Asc, SizeOf, A, B, C>;   // HTList<SizeOf<B>, SizeOf<C>, SizeOf<A>>
using SortedTuple    = typename to_tuple<SortedWrappers>::type; // std::tuple<B, C, A>

static_assert(std::is_same_v<SortedTuple, std::tuple<B, C, A>>);

// Sort by alignof descending:
using AlignSorted = TypeSort<Order::Desc, AlignOf, A, B, C>;
using AlignTuple  = typename to_tuple<AlignSorted>::type; // order depends on alignof(...)
```

### 2) Partition once, get both buckets

Useful for small-buffer optimization (inline vs heap), MTU budgets, feature gates, etc.

```cpp
// Threshold/pivot type:
struct Inline32 { static constexpr std::size_t size = 32; };

// Build wrapped set:
using Set = HTList<SizeOf<A>, SizeOf<B>, SizeOf<C>>;

// One pass: pass = <= 32B, fail = > 32B
using P = partition_by<Inline32, leq_size, SizeOf<A>, SizeOf[B], SizeOf<C]>;

using FitsInlineWrappers = typename P::pass; // HTList<SizeOf<B>, SizeOf<C> >
using NeedsHeapWrappers  = typename P::fail; // HTList<SizeOf<A>>

// Project to convenient shapes:
using FitsInline = typename to_variant<FitsInlineWrappers>::type; // std::variant<B, C>
using NeedsHeap  = typename to_variant<NeedsHeapWrappers>::type;  // std::variant<A>
```

### 3) Reduce (sum/min/max) over the key

```cpp
// Predefined reducers: add_i, min_i, max_i
using TotalBytes = reduce_sizes_t<add_i, 0, SizeOf<A>, SizeOf<B>, SizeOf<C>>;
static_assert(TotalBytes::value == sizeof(A) + sizeof(B) + sizeof(C));

// Convenience alias:
template <typename... Ts>
using Sum = reduce_sizes_t<add_i, 0, Ts...>;
static_assert(Sum<SizeOf<A>, SizeOf<B>>::value == sizeof(A) + sizeof(B));
```

---

## Core API (cheat sheet)

* **Type list**

  * `template<class... Ts> struct HTList;`
  * `append<HTList<Xs...>, HTList<Ys...>>::type  // HTList<Xs..., Ys...>`
  * `to_tuple<HTList<Wrap<Ts>...>>::type         // std::tuple<Ts...>`
  * `to_variant<HTList<Wrap<Ts>...>>::type       // std::variant<Ts...>`

* **Sort**

  * `enum class Order { Asc, Desc };`
  * `TypeSort<Order, KeyOf, Ts...>` → `HTList<KeyOf<Ts>...>` sorted by `KeyOf<T>::size`

* **Partition**

  * `partition_by<Pivot, Rel, Ts...>` → `{ using pass, using fail; }`
  * `filter_by<Pivot, Rel, Ts...>`    → `pass`
  * `reject_if_by<Pivot, Rel, Ts...>` → `fail`
  * `Rel<Pivot, Elem>::value` is any binary relation you define (e.g. `leq_size`)

* **Reduce**

  * `reduce_sizes_t<Op, Init, Ts...>` → `std::integral_constant<std::size_t, ...>`
  * Built-ins: `add_i`, `min_i`, `max_i`

> ℹ️ A **KeyOf** is any `template<class T> struct KeyOf { using type = T; static constexpr std::size_t size = ...; };`

---

## Minimal demo (copy/paste)

```cpp
#include <ctql/ctql.hpp>
#include <tuple>
#include <type_traits>
using namespace ctql;

struct X { double d[2]; }; // 16B
struct Y { int    i;    }; // 4B
struct Z { char   c[3]; }; // 3B

template <class T> struct SizeOf { using type = T; static constexpr std::size_t size = sizeof(T); };
template <class P, class E> struct leq_size { static constexpr bool value = (E::size <= P::size); };

int main() {
  using Sorted = TypeSort<Order::Asc, SizeOf, X, Y, Z>;
  using Tup    = typename to_tuple<Sorted>::type;
  static_assert(std::is_same_v<Tup, std::tuple<Z, Y, X>>);

  struct Cap8 { static constexpr std::size_t size = 8; };
  using P = partition_by<Cap8, leq_size, SizeOf<X>, SizeOf<Y>, SizeOf<Z>>;
  using Small = typename to_tuple<typename P::pass>::type; // <= 8B
  using Big   = typename to_tuple<typename P::fail>::type; // > 8B
  static_assert(std::is_same_v<Small, std::tuple<Y, Z>>);
  static_assert(std::is_same_v<Big,   std::tuple<X>>);

  using Total = reduce_sizes_t<add_i, 0, SizeOf<X>, SizeOf<Y>, SizeOf<Z>>;
  static_assert(Total::value == sizeof(X)+sizeof(Y)+sizeof(Z));
}
```

Build:

```bash
clang++-18 -std=c++23 demo.cpp -Iinclude
```

---

## Notes

* Works with any “size” metric you provide: `sizeof`, `alignof`, protocol bytes, priorities, etc.
* Sorting uses a pivot-partition strategy; equals go to the “right” bucket by default (deterministic, not stable).
* Everything is `constexpr`/type-level; no runtime overhead.

---

If you want a fancier DSL (e.g., `"lt"_op` tags), `ctql` ships tiny operator tags & UDLs too—keep an eye on `predicate.hpp` inside the header.
