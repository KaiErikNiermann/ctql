# ctql — Compile-Time Query Language (for types)

Header-only C++23 toolkit to **filter/partition/sort/reduce *types*** using user-defined keys (like `sizeof(T)`, `alignof(T)`, or your own compile-time metric). Also includes tiny `ct_string` helpers and nicer concept/predicate wrappers.

* **Single header:** `#include <ctql/ctql.hpp>`
* **Standard:** C++23
* **Compiler:** Clang++ 18 (tested)

--- 
## Examples 

For examples see [here](https://github.com/KaiErikNiermann/ctql/tree/main/examples). 

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

## Notes

* Works with any “size” metric you provide: `sizeof`, `alignof`, protocol bytes, priorities, etc.
* Sorting uses a pivot-partition strategy; equals go to the “right” bucket by default (deterministic, not stable).
* Everything is `constexpr`/type-level; no runtime overhead.
