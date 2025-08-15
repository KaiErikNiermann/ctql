
.. _program_listing_file_include_htlist.hpp:

Program Listing for File htlist.hpp
===================================

|exhale_lsh| :ref:`Return to documentation for file <file_include_htlist.hpp>` (``include/htlist.hpp``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

   #pragma once
   
   #include <cstddef>
   namespace ctql {
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
   }
