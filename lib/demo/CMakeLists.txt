add_executable(mathutils_demo main.cpp)

target_link_libraries(mathutils_demo
    PRIVATE
        mathutils::mathutils
        mathutils::warnings)
