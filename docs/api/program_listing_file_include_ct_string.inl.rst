
.. _program_listing_file_include_ct_string.inl:

Program Listing for File ct_string.inl
======================================

|exhale_lsh| :ref:`Return to documentation for file <file_include_ct_string.inl>` (``include/ct_string.inl``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

   #pragma once
   
   #include <array>
   #include <cstddef>
   #include <string_view>
   
   namespace ctql {
       template <size_t N>
       struct ct_string {
           std::array<char, N + 1> data {};
   
           constexpr ct_string() = default;
   
           constexpr ct_string(const char (&str)[N + 1]) {
               for (size_t i = 0; i < N + 1; ++i)
                   data[i] = str[i];
           }
   
           constexpr const char* c_str() const { return data.data(); }
   
           constexpr std::size_t size() const { return N; }
   
           constexpr operator std::string_view() const { return {data.data(), N}; }
       };
   
       template <std::size_t N>
       ct_string(const char (&)[N]) -> ct_string<N - 1>;
   
       template <size_t N1, size_t N2>
       constexpr auto operator+(const ct_string<N1>& lhs, const ct_string<N2>& rhs) {
           ct_string<N1 + N2> result;
           for (size_t i = 0; i < N1; ++i)
               result.data[i] = lhs.data[i];
           for (size_t i = 0; i < N2; ++i)
               result.data[N1 + i] = rhs.data[i];
           result.data[N1 + N2] = '\0';
           return result;
       }
   
       template <size_t N1, size_t N2>
       constexpr bool operator==(const ct_string<N1>& lhs, const ct_string<N2>& rhs) {
           if constexpr (N1 != N2)
               return false;
           else {
               for (size_t i = 0; i < N1; ++i) {
                   if (lhs.data[i] != rhs.data[i])
                       return false;
               }
               return true;
           }
       }
   
       consteval std::size_t count_digits(std::size_t n) {
           std::size_t digits = 1;
           while (n /= 10)
               ++digits;
           return digits;
       }
   
       template <std::size_t N>
       consteval auto to_ct_string() {
           constexpr std::size_t digits = count_digits(N);
           ct_string<digits> result;
           std::size_t n = N;
           for (std::size_t i = 0; i < digits; ++i)
               result.data[digits - 1 - i] = '0' + (n % 10), n /= 10;
           result.data[digits] = '\0';
           return result;
       }
   
       template <ct_string str>
       constexpr auto operator""_ct() {
           return str;
       }
   
       template <typename... Lambda_t>
       struct match : Lambda_t... {
           using Lambda_t::operator()...;
           constexpr match(Lambda_t... lambda)
               : Lambda_t(std::move(lambda))... { }
       };
   
   #define $match(arg, ...) std::invoke(ctql::match {__VA_ARGS__}, arg)
   
       template <class... Ts, class Fn>
       constexpr void foreach_indexed(Fn&& fn) {
           [&]<std::size_t... Is>(std::index_sequence<Is...>) {
               (std::forward<Fn>(fn).template operator()<Ts, Is>(), ...);
           }(std::index_sequence_for<Ts...> {});
       }
   }
