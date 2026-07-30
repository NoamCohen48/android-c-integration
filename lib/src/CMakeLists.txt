# ---------------------------------------------------------------------------
# The mathutils library target, assembled from its components.
# ---------------------------------------------------------------------------
add_library(mathutils
    arithmetic/basic.cpp
    combinatorics/factorial.cpp
    primes/primality.cpp
    primes/sieve.cpp)
add_library(mathutils::mathutils ALIAS mathutils)

# Generate the version header from the project() version.
configure_file(
    ${PROJECT_SOURCE_DIR}/include/mathutils/version.hpp.in
    ${PROJECT_BINARY_DIR}/generated/mathutils/version.hpp
    @ONLY)

target_include_directories(mathutils
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/generated>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_compile_features(mathutils PUBLIC cxx_std_17)

target_link_libraries(mathutils PRIVATE mathutils::warnings)

set_target_properties(mathutils PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR})
