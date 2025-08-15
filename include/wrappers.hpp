#pragma once

#include <complex>
#include <cstdint>
#include <functional>
#include <map>
#include <set>
#include <type_traits>
#include <variant>
#include <typedefs.inl>

namespace ctql {
    template <typename T>
    concept is_bits_array
        = std::is_trivially_copyable_v<typename T::value_type> && std::is_trivially_copyable_v<T>;

    /// @concept is std::complex
    template <typename T>
    concept is_complex = requires(T t) {
        typename T::value_type;
        std::is_same_v<T, std::complex<typename T::value_type>>;
    };

    /// @concept is std::vector
    template <typename T>
    concept is_vector = requires(T t) {
        typename T::value_type;
        std::is_same_v<T, std::vector<typename T::value_type>>;
    };

    /// @concept is map
    template <typename T>
    concept is_map = requires(T t) {
        typename T::key_type;
        typename T::mapped_type;
        std::is_same_v<T, std::map<typename T::key_type, typename T::mapped_Type>>
            or std::is_same_v<T, std::unordered_map<typename T::key_type, typename T::mapped_Type>>
            or std::is_same_v<T, std::multimap<typename T::key_type, typename T::mapped_Type>>;
    };

    /// @concept is std::set
    template <typename T>
    concept is_set = requires(T t) {
        typename T::value_type;
        std::is_same_v<T, std::set<typename T::value_type>>;
    };

    /// @concept is pair
    template <typename T>
    concept is_pair = requires(T) {
        std::is_same_v<T, std::pair<typename T::first_type, typename T::second_type>>;
    };

    /// @concept is tuple
    namespace detail {
        template <typename T, uint64_t... Ns>
        constexpr bool is_tuple_aux(std::index_sequence<Ns...>) {
            return std::is_same_v<T, std::tuple<std::tuple_element_t<Ns, T>...>>;
        }
    } // namespace detail

    template <typename T>
    struct is_std_array : std::false_type { };

    template <typename T, std::size_t N>
    struct is_std_array<std::array<T, N>> : std::true_type { };

    template <typename T>
    concept is_array = requires(T t) {
        typename T::value_type;
        std::is_same_v<T, std::array<typename T::value_type, std::tuple_size<T>::value>>;
    };

    template <typename T>
    concept is_tuple
        = detail::is_tuple_aux<T>(std::make_index_sequence<std::tuple_size<T>::value>());

    /// @concept: wrapper for std::is_same_v
    template <class T, class U>
    struct is_same_or_const {
        static constexpr bool value = false;
    };

    template <class T>
    struct is_same_or_const<T, T> {
        static constexpr bool value = true;
    };

    template <class T>
    struct is_same_or_const<T, const T> {
        static constexpr bool value = true;
    };

    template <typename T, typename U>
    concept is_same = std::is_same_v<T, U>;

#define $type_eq(T1, T2) std::is_same_v<T1, T2>

    template <typename T, typename U>
    concept is_not_same = not is_same<T, U>;

    template <typename T, typename U>
    concept is_conv_to = std::is_convertible_v<T, U>;

    template <typename T, typename U>
    concept is_not_conv_to = not is_conv_to<T, U>;

    template <typename T, typename U>
    concept is
        = std::conditional_t<std::is_void_v<T>, std::is_void<U>, is_same_or_const<T, U>>::value;

    template <typename T, typename U>
    concept is_not = not is<T, U>;

    /// @brief introspect functions
    template <typename T>
    struct function_traits;

    template <typename Return_t, typename... Args_t>
    struct function_traits<Return_t(Args_t...)> {
        // as C literal function
        using as_c_function = Return_t(Args_t...);

        // as std::function
        using as_std_function = std::function<Return_t(Args_t...)>;

        // return type
        using return_t = typename as_std_function::result_type;

        // number of arguments
        static constexpr uint64_t n_args = sizeof...(Args_t);

        // all argument types as tuple type
        using argument_ts = std::tuple<Args_t...>;

        // type of i-th argument
        template <uint64_t i>
        using argument_type = std::tuple_element_t<i, argument_ts>;
    };

    template <typename T, uint64_t i>
    using get_nth_argument_t = std::tuple_element_t<i, typename function_traits<T>::argument_ts>;

    /// @brief forward function or lambda as C-literal function
    namespace detail {
        template <typename T>
        struct as_c_function;

        template <typename Return_t, typename... Args_t>
        struct as_c_function<Return_t(Args_t...)> {
            using value = Return_t(Args_t...);
        };

        template <typename Return_t, typename... Args_t>
        struct as_c_function<std::function<Return_t(Args_t...)>> {
            using value = Return_t(Args_t...);
        };

        template <typename T>
        using as_c_function_v = typename as_c_function<T>::value;
    } // namespace detail

    /// @concept: check signature of argument function or lambda
    template <typename T, typename Return_t, typename... Args_t>
    concept is_function_with_signature
        = std::is_invocable_v<detail::as_c_function_v<T>, Args_t...>
          and std::conditional_t<
              std::is_void_v<Return_t>,
              std::is_void<typename function_traits<detail::as_c_function_v<T>>::return_t>,
              std::is_same<
                  typename function_traits<detail::as_c_function_v<T>>::return_t,
                  Return_t>>::value;

    template <class T>
    concept HasStaticSize = requires {
        { T::size } -> std::convertible_to<std::size_t>;
    };

    template <typename T>
    struct Size {
        using type = T;
        static constexpr std::size_t size = T::size;
    };


    template <typename Cmp, HasStaticSize A, HasStaticSize B>
    struct cmp {
        static constexpr bool value
            = Cmp {}(static_cast<std::size_t>(A::size), static_cast<std::size_t>(B::size));
    };

    template <typename Cmp>
    struct PredBy {
        template <HasStaticSize Pivot, HasStaticSize Elem>
        struct t {
            static constexpr bool value = cmp<Cmp, Elem, Pivot>::value;
        };
    };

    struct ops {
        template <typename L, typename R>
        using leq = PredBy<std::less_equal<>>::template t<L, R>;
        
        template <typename L, typename R>
        using geq = PredBy<std::greater_equal<>>::template t<L, R>;
        
        template <typename L, typename R>
        using lt = PredBy<std::less<>>::template t<L, R>;
        
        template <typename L, typename R>
        using gt = PredBy<std::greater<>>::template t<L, R>;
        
        template <typename L, typename R>
        using eq = PredBy<std::equal_to<>>::template t<L, R>;
    };

    template <typename Cmp>
    struct op_tag {
        template <typename L, typename R>
        using pred = typename PredBy<Cmp>::template t<L, R>;
    };

    template <ct_string str>
    using op_type = std::conditional_t<
        str == "<="_ct, op_tag<std::less_equal<>>,
        std::conditional_t<
            str == ">="_ct, op_tag<std::greater_equal<>>,
        std::conditional_t<
            str == "<"_ct, op_tag<std::less<>>,
        std::conditional_t<
            str == ">"_ct, op_tag<std::greater<>>,
        std::conditional_t<
            str == "=="_ct, op_tag<std::equal_to<>>, void>
        >
    >>>;

#define $op(str) ctql::op_type<str##_ct>::template pred

    /// @concept: function with n args
    template <typename T, uint64_t N>
    concept is_function_with_n_args = (function_traits<detail::as_c_function_v<T>>::n_args == N);

    /// @concept: heterogeneous type list
    namespace detail {
        template <typename... T>
        struct HTList;

        template <>
        struct HTList<> {
            static constexpr std::size_t len = 0;
        };

        template <typename T0, typename... TRest>
        struct HTList<T0, TRest...> : HTList<TRest...> {
            using head_type = T0;
            using tail_type = HTList<TRest...>;

            static constexpr std::size_t len = 1 + sizeof...(TRest);
        };

        template <typename A, typename U>
        struct append {
            using type = U;
        };

        template <typename T0, typename... TRest, typename... Us>
        struct append<HTList<T0, TRest...>, HTList<Us...>> {
            using type = HTList<T0, TRest..., Us...>;
        };

        template <typename... T>
        HTList(T...) -> HTList<T...>;
    } // namespace detail

#define $type_list(...) detail::HTList<__VA_ARGS__>

    namespace detail {
        template <typename Pivot, template <typename, typename> class Rel, typename List>
        struct partition_by_impl;

        template <typename Pivot, template <typename, typename> class Rel>
        struct partition_by_impl<Pivot, Rel, HTList<>> {
            using pass = HTList<>; // elements where Rel<Pivot, Elem> is true
            using fail = HTList<>; // the rest
        };

        template <typename Pivot, template <typename, typename> class Rel, typename H, typename... Ts>
        struct partition_by_impl<Pivot, Rel, HTList<H, Ts...>> {
        private:
            using rest = partition_by_impl<Pivot, Rel, HTList<Ts...>>;
        public:
            using pass = std::conditional_t<
                Rel<Pivot, H>::value,
                typename append<HTList<H>, typename rest::pass>::type,
                typename rest::pass
            >;
            using fail = std::conditional_t<
                Rel<Pivot, H>::value,
                typename rest::fail,
                typename append<HTList<H>, typename rest::fail>::type
            >;
        };
    } // namespace detail

    template <typename Pivot, template <typename, typename> class Rel, typename... Ts>
    struct partition_by {
        using impl = detail::partition_by_impl<Pivot, Rel, detail::HTList<Ts...>>;
        using pass = typename impl::pass; // “kept” / left bucket
        using fail = typename impl::fail; // “rejected” / right bucket
        using concat = typename detail::HTList<pass, fail>; 
    };

    template <typename Pivot, template <typename, typename> class Rel, typename... Ts>
    using filter_by  = typename partition_by<Pivot, Rel, Ts...>::pass;

    template <typename Pivot, template <typename, typename> class Rel, typename... Ts>
    using reject_if_by = typename partition_by<Pivot, Rel, Ts...>::fail;

    template <typename Pivot, template <typename, typename> class Rel, template <typename> class KeyOf = Size, typename... Ts>
    using partition_by_key = partition_by<Pivot, Rel, KeyOf<Ts>...>;

#define $partition_by(Pivot, Rel, ...) \
    typename ctql::partition_by<Pivot, Rel, __VA_ARGS__>::concat

#define $partition_by_key(KeyOf, Pivot, Rel , ...) \
   ctql::partition_by_key<Pivot, Rel, KeyOf, __VA_ARGS__>    

#define $size(n) static constexpr std::size_t size = n;

    enum class Order { Asc, Desc };

    template <Order Ord, typename List>
    struct sort_list;

    template <Order Ord>
    struct sort_list<Ord, detail::HTList<>> {
        using type = detail::HTList<>;
    };

    template <Order Ord, HasStaticSize T0, HasStaticSize... TRest>
        struct sort_list<Ord, detail::HTList<T0, TRest...>> {
        private:
        using LeftRel  = std::conditional_t<Ord == Order::Asc,
                            op_type<"<"_ct>,  // Elem < Pivot
                            op_type<">"_ct>>; // Elem > Pivot
        using RightRel =  std::conditional_t<Ord == Order::Asc,
                            op_type<">="_ct>, // Elem >= Pivot
                            op_type<"<="_ct>>; // Elem <= Pivot

        using P = partition_by<T0, LeftRel::template pred, TRest...>; // pass = Left, fail = not-left
        using LeftSorted  = typename sort_list<Ord, typename P::pass>::type;

        using RightUnsorted = filter_by<T0, RightRel::template pred, TRest...>;
        using RightSorted   = typename sort_list<Ord, RightUnsorted>::type;

        public:
        using type = typename detail::append<typename detail::append<LeftSorted, detail::HTList<T0>>::type, RightSorted>::type;
    };

    template <Order Ord = Order::Asc, template <typename> class KeyOf = Size, typename... Ts>
    using TypeSort = typename sort_list<Ord, detail::HTList<KeyOf<Ts>...>>::type;

    /// @brief compile-time fold 
    namespace detail {
        template <typename List, template <std::size_t, std::size_t> class Op, std::size_t Acc>
        struct fold_sizes;

        template <template <std::size_t, std::size_t> class Op, std::size_t Acc>
        struct fold_sizes<HTList<>, Op, Acc> : std::integral_constant<std::size_t, Acc> { };

        template <
            HasStaticSize H,
            typename... Ts,
            template <std::size_t, std::size_t> class Op,
            std::size_t Acc>
        struct fold_sizes<HTList<H, Ts...>, Op, Acc>
            : fold_sizes<HTList<Ts...>, Op, Op<Acc, static_cast<std::size_t>(H::size)>::value> { };

        
        // ---------- size operations ----------
        template <std::size_t A, std::size_t B>
        struct add_i : std::integral_constant<std::size_t, A + B> { };

        template <std::size_t A, std::size_t B>
        struct min_i : std::integral_constant<std::size_t, (A < B ? A : B)> { };

        template <std::size_t A, std::size_t B>
        struct max_i : std::integral_constant<std::size_t, (A < B ? B : A)> { };
        // -------------------------------------
    } // namespace detail

    template <template <std::size_t, std::size_t> class Op, std::size_t Init, typename... Ts>
    using reduce_sizes_t = detail::fold_sizes<detail::HTList<Ts...>, Op, Init>;

    template <template <std::size_t, std::size_t> class Op, std::size_t Init, typename... Ts>
    inline constexpr std::size_t reduce_sizes_v = reduce_sizes_t<Op, Init, Ts...>::value;

    template <typename... Ts>
    using Sum = reduce_sizes_t<detail::add_i, 0, Ts...>;

    template <typename... Ts>
    inline constexpr std::size_t Sum_v = Sum<Ts...>::value;

#define $reduce_sizes(...) (reduce_sizes_v<__VA_ARGS__>)
#define $sum_sizes(...) (Sum_v<__VA_ARGS__>)

    // ---------- extraction helpers ----------
    template <typename>
    struct to_variant;
    template <typename... Ms>
    struct to_variant<detail::HTList<Ms...>> {
        using type = std::variant<typename Ms::type...>;
    };

    template <typename>
    struct to_tuple;
    template <typename... Ms>
    struct to_tuple<detail::HTList<Ms...>> {
        using type = std::tuple<typename Ms::type...>;
    };

#define $to_variant(...) typename ctql::to_variant<__VA_ARGS__>::type
#define $to_tuple(...) typename ctql::to_tuple<__VA_ARGS__>::type
#define $tuple_t(...) std::tuple<__VA_ARGS__>

#define $filter_by(T, Pred, ...)  \
    ctql::filter_by<T, Pred, __VA_ARGS__>

#define $sort_types(...)                                                                    \
    $to_tuple(ctql::TypeSort<ctql::Order::Asc, Size, __VA_ARGS__>)

#define $sort_types_by(KeyOf, ...)                                                                  \
    $to_tuple(ctql::TypeSort<ctql::Order::Asc, KeyOf, __VA_ARGS__>)

#define $sort_types_desc(KeyOf, ...)                                                            \
    $to_tuple(ctql::TypeSort<ctql::Order::Desc, KeyOf, __VA_ARGS__>)

}