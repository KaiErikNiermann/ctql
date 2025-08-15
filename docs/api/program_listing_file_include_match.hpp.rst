
.. _program_listing_file_include_match.hpp:

Program Listing for File match.hpp
==================================

|exhale_lsh| :ref:`Return to documentation for file <file_include_match.hpp>` (``include/match.hpp``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: cpp

   // ctql/match.hpp (or inside your header)
   
   #include <type_traits>
   
   namespace ctql {
   
       template <auto K, class T>
       struct case_ {
           static constexpr auto key = K;
           using type                = T;
       };
   
       // An optional default if nothing matches
       template <class T>
       struct default_ {
           using type = T;
       };
   
       namespace detail {
           // -------- matcher core (first-match wins; default if no match) --------
           template <auto Key, class Default, bool Matched, class... Alts>
           struct match_impl;
   
           // end of list → yield accumulated Default
           template <auto Key, class Default, bool Matched>
           struct match_impl<Key, Default, Matched> {
               using type = Default;
           };
   
           // case_ step: if not matched yet and Key == K, lock in T; otherwise keep going
           template <auto Key, class Default, bool Matched, auto K, class T, class... Rest>
           struct match_impl<Key, Default, Matched, case_<K, T>, Rest...>
               : match_impl<
                     Key,
                     std::conditional_t<!Matched && (Key == K), T, Default>,
                     (Matched || (Key == K)),
                     Rest...> { };
   
           // default_ step: if not matched yet, update fallback; otherwise ignore
           template <auto Key, class Default, bool Matched, class T, class... Rest>
           struct match_impl<Key, Default, Matched, default_<T>, Rest...>
               : match_impl<Key, std::conditional_t<Matched, Default, T>, Matched, Rest...> { };
   
           // Public interface: match_t<Key, Alts...> → resulting type
           template <auto Key, class... Alts>
           struct match {
               using type = typename match_impl<Key, void, false, Alts...>::type;
           };
       }
   
       template <auto Key, class... Alts>
       using match_t = typename detail::match<Key, Alts...>::type;
   
   } // namespace ctql
