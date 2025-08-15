.. _exhale_struct_structctql_1_1detail_1_1match__impl_3_01Key_00_01Default_00_01Matched_00_01case___3_01K_00_01T_01_4_00_01Rest_8_8_8_01_4:

Template Struct match_impl< Key, Default, Matched, case_< K, T >, Rest... >
===========================================================================

- Defined in :ref:`file_include_match.hpp`


Inheritance Relationships
-------------------------

Base Type
*********

- ``public ctql::detail::match_impl< Key, std::conditional_t<!Matched &&(Key==K), T, Default >,(Matched||(Key==K)), Rest... >`` (:ref:`exhale_struct_structctql_1_1detail_1_1match__impl`)


Struct Documentation
--------------------


.. doxygenstruct:: ctql::detail::match_impl< Key, Default, Matched, case_< K, T >, Rest... >
   :project: ctql
   :members:
   :protected-members:
   :undoc-members: