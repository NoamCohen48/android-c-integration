add_executable(mathutils_tests mathutils_tests.cpp)

target_link_libraries(mathutils_tests
    PRIVATE
        mathutils::mathutils
        mathutils::warnings)

add_test(NAME mathutils_tests COMMAND mathutils_tests)
