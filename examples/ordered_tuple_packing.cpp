#include <cstddef>
#include <ctql.hpp>

// ---- metric wrapper: you decide what "size" means
template <class T>
struct SizeTagged {
  using type = T;
  static constexpr std::size_t size = sizeof(T);        // or alignof(T)
};

// ---- example payload types
struct Small  { char c[1];  };
struct Medium { alignas(8) char c[9]; };
struct Big    { double x[4]; };

static_assert($type_eq(
    $sort_types_by(SizeTagged, Big, Small, Medium), 
    $tuple_t(Small, Medium, Big)) 
);
