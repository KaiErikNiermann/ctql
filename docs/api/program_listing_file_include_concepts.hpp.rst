
.. _program_listing_file_include_concepts.hpp:

Program Listing for File concepts.hpp
=====================================

|exhale_lsh| :ref:`Return to documentation for file <file_include_concepts.hpp>` (``include/concepts.hpp``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

   #pragma once
   
   #include "htlist.hpp"
   #include <complex>
   #include <cstdint>
   #include <functional>
   #include <map>
   #include <set>
   #include <type_traits>
   #include <variant>
   
   namespace ctql {
       template <typename T>
       concept is_bits_array
           = std::is_trivially_copyable_v<typename T::value_type> && std::is_trivially_copyable_v<T>;
   
       template <typename T>
       concept is_complex = requires(T t) {
           typename T::value_type;
           std::is_same_v<T, std::complex<typename T::value_type>>;
       };
   
       template <typename T>
       concept is_vector = requires(T t) {
           typename T::value_type;
           std::is_same_v<T, std::vector<typename T::value_type>>;
       };
   
       template <typename T>
       concept is_map = requires(T t) {
           typename T::key_type;
           typename T::mapped_type;
           std::is_same_v<T, std::map<typename T::key_type, typename T::mapped_Type>>
               or std::is_same_v<T, std::unordered_map<typename T::key_type, typename T::mapped_Type>>
               or std::is_same_v<T, std::multimap<typename T::key_type, typename T::mapped_Type>>;
       };
   
       template <typename T>
       concept is_set = requires(T t) {
           typename T::value_type;
           std::is_same_v<T, std::set<typename T::value_type>>;
       };
   
       template <typename T>
       concept is_pair = requires(T) {
           std::is_same_v<T, std::pair<typename T::first_type, typename T::second_type>>;
       };
   
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
   
       template <typename T, typename Return_t, typename... Args_t>
       concept is_function_with_signature
           = std::is_invocable_v<detail::as_c_function_v<T>, Args_t...>
             and std::conditional_t<
                 std::is_void_v<Return_t>,
                 std::is_void<typename function_traits<detail::as_c_function_v<T>>::return_t>,
                 std::is_same<
                     typename function_traits<detail::as_c_function_v<T>>::return_t,
                     Return_t>>::value;
   
   
                     
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
   }
