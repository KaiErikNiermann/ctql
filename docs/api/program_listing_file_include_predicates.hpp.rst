
.. _program_listing_file_include_predicates.hpp:

Program Listing for File predicates.hpp
=======================================

|exhale_lsh| :ref:`Return to documentation for file <file_include_predicates.hpp>` (``include/predicates.hpp``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

   #pragma once
   #include "htlist.hpp"
   #include <concepts>
   #include <functional>
   #include <ct_string.inl>
   #include <match.hpp>
   
   namespace ctql {
       template <class T>
       concept HasStaticSize = requires {
           { T::size } -> std::convertible_to<std::size_t>;
       };
   
       template <typename T>
       struct Size {
           using type                        = T;
           static constexpr std::size_t size = T::size;
       };
   
       template <typename T>
       struct SizeOf {
           using type                        = T;
           static constexpr std::size_t size = sizeof(T);
       };
   
       template <typename T>
       struct AlignOf {
           using type                        = T;
           static constexpr std::size_t size = alignof(T);
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
   
       
       template <typename Cmp>
       struct op_tag {
           template <typename L, typename R>
           using pred = typename PredBy<Cmp>::template t<L, R>;
       };
     
       struct ops {
           using leq = op_tag<std::less_equal<>>;
           using geq = op_tag<std::greater_equal<>>;
           using lt  = op_tag<std::less<>>;
           using gt  = op_tag<std::greater<>>;
           using eq  = op_tag<std::equal_to<>>;
           using neq = op_tag<std::not_equal_to<>>;
       };
   
      template <ct_string str>
       using op_type = ctql::match_t< 
           str,
           case_<"<="_ct, ops::leq>,
           case_<"<"_ct, ops::lt>,
           case_<">="_ct, ops::geq>,
           case_<">"_ct, ops::gt>,
           case_<">="_ct, ops::geq>,
           case_<"=="_ct, ops::eq>,
           case_<"!="_ct, ops::neq>,
           default_<void>  
       >;
                       
       template <typename... Ts>
       using Size_t = detail::HTList<Size<Ts>...>;
       
       template <typename... Ts>
       using SizeOf_t = detail::HTList<SizeOf<Ts>...>;
       
       template <typename... Ts>
       using AlignOf_t = detail::HTList<AlignOf<Ts>...>;
       
       template <template <typename> class S, typename... Ts>
       using Apply = detail::HTList<S<Ts>...>;
   }
