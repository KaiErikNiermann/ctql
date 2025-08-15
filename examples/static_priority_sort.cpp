#include <iostream>
#include <ctql.hpp>

using namespace ctql;

// ---- pretend "size" encodes priority (lower = earlier)
struct Parse    { $size(10); static void run(){ std::cout<<"parse\n"; } };
struct Validate { $size(20); static void run(){ std::cout<<"validate\n"; } };
struct Log      { $size(5); static void run(){ std::cout<<"log\n"; } };

template <class T> struct Step { using type = T; $size(T::size); };

template <typename> struct to_array;
template <typename... Ms>
struct to_array<$tuple_t(Ms...)> {
    static void run_all() { (Ms::run(), ...); }
};

using StepsSorted = $sort_types(Log, Parse, Validate);

static_assert($type_eq(StepsSorted, $tuple_t(Log, Parse, Validate)));

int main() {
  to_array<StepsSorted>::run_all(); // prints: log\nparse\nvalidate\n
}
