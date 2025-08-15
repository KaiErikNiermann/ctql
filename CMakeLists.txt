cmake_minimum_required(VERSION 3.21) # for modern target_include_directories usage
project(ctql
    VERSION 0.1.0
    DESCRIPTION "Compile-time programming utilities for C++"
    LANGUAGES CXX
)

# Create an INTERFACE target (no compilation, just properties)
add_library(ctql INTERFACE)
# Optional: add an ALIAS for namespaced usage in-tree
add_library(ctql::ctql ALIAS ctql)

# Tell consumers where to find headers
target_include_directories(ctql
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>          # for <ctql.hpp> at repo root
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>  # for headers under include/
        $<INSTALL_INTERFACE:include>
)

# Require at least C++20 (or 23 if you want)
target_compile_features(ctql INTERFACE cxx_std_23)

# Optional: export a version macro so users can query ctql_VERSION
target_compile_definitions(ctql
    INTERFACE
        ctql_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        ctql_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        ctql_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)

# ===================
# Installation setup
# ===================
include(GNUInstallDirs)

# Install headers
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
# Also install the umbrella header that lives at the repo root
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/ctql.hpp DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Install the target
install(TARGETS ctql EXPORT ctqlTargets)

# Export target info for find_package
install(EXPORT ctqlTargets
    FILE ctqlTargets.cmake
    NAMESPACE ctql::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ctql
)

include(CTest)
### Declare Test ###

option(BUILD_TESTING "Enable ctql_test" ON)
if (BUILD_TESTING)
    enable_testing()
    add_executable(
        ctql_test
        tests/main.cpp
    )
    # Link the interface target so include dirs propagate to the test
    target_link_libraries(ctql_test PRIVATE ctql::ctql)
    add_test(NAME ctql_test COMMAND ctql_test)
endif()

option(BUILD_EXAMPLES "Enable ctql examples" ON)
if (BUILD_EXAMPLES)
    add_executable(
        static_priority_sort_example
        examples/static_priority_sort.cpp
    )
    # Link the interface target so include dirs propagate to the example
    target_link_libraries(static_priority_sort_example PRIVATE ctql::ctql)
endif()

# Create a basic Config and Version file for find_package
include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/ctqlConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ctqlConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/ctqlConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ctql
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/ctqlConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/ctqlConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ctql
)
