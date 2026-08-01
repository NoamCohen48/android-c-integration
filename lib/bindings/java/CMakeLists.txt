cmake_minimum_required(VERSION 3.22)

project(mathutils_jni LANGUAGES CXX)

# ---------------------------------------------------------------------------
# The Java/JNI binding for mathutils. It lives with the library rather than
# with any particular consumer, because everything it describes and produces --
# the SWIG interface, the Java proxies, the JNI wrapper -- is plain,
# app-agnostic Java/C++.
#
#   mathutils.i        the hand-written SWIG interface
#   generated/java/    SWIG output: proxies in package com.example.mathutils
#   generated/cpp/     SWIG output: mathutils_wrap.cxx, compiled here
#
# Standalone (host) build -- generates and compiles everything itself:
#
#   cmake -S lib/bindings/java -B lib/bindings/java/build
#   cmake --build lib/bindings/java/build
#
# Android build: the app's Gradle build runs SWIG once (see :app:generateSwig)
# and configures this project per ABI with MATHUTILS_JAVA_RUN_SWIG=OFF, so the
# parallel per-ABI configures do not race regenerating the same files.
# ---------------------------------------------------------------------------
set(MATHUTILS_JAVA_PACKAGE "com.example.mathutils"
    CACHE STRING "Java package for the generated proxy classes")
set(MATHUTILS_JAVA_GENERATED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/generated"
    CACHE PATH "Where the SWIG-generated Java and C++ are written/read")
option(MATHUTILS_JAVA_RUN_SWIG
    "Run SWIG as part of this build (off when a driving build generates it)" ON)

string(REPLACE "." "/" _mathutils_java_pkg_path "${MATHUTILS_JAVA_PACKAGE}")
set(MATHUTILS_JAVA_OUT_DIR "${MATHUTILS_JAVA_GENERATED_DIR}/java/${_mathutils_java_pkg_path}")
set(MATHUTILS_JAVA_WRAP_CXX "${MATHUTILS_JAVA_GENERATED_DIR}/cpp/mathutils_wrap.cxx")

# Reuse the mathutils library unchanged (no demo/tests in a bindings build).
# The static library gets linked into a shared object, so it needs PIC (the NDK
# compiles PIC anyway; this is what makes a host build work too).
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(MATHUTILS_BUILD_DEMO OFF CACHE BOOL "" FORCE)
set(MATHUTILS_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../.. ${CMAKE_BINARY_DIR}/mathutils)

set(_mathutils_include_dir "${CMAKE_CURRENT_SOURCE_DIR}/../../include")

if(MATHUTILS_JAVA_RUN_SWIG)
    find_package(SWIG 4.0 REQUIRED)

    # The Java proxies are a by-product of the same invocation; listing them as
    # BYPRODUCTS lets consumers depend on them without a second SWIG run.
    add_custom_command(
        OUTPUT ${MATHUTILS_JAVA_WRAP_CXX}
        BYPRODUCTS ${MATHUTILS_JAVA_OUT_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${MATHUTILS_JAVA_OUT_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${MATHUTILS_JAVA_GENERATED_DIR}/cpp
        COMMAND ${SWIG_EXECUTABLE} -c++ -java
                -package ${MATHUTILS_JAVA_PACKAGE}
                -I${_mathutils_include_dir}
                -outdir ${MATHUTILS_JAVA_OUT_DIR}
                -o ${MATHUTILS_JAVA_WRAP_CXX}
                ${CMAKE_CURRENT_SOURCE_DIR}/mathutils.i
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/mathutils.i
        COMMENT "Generating Java bindings with SWIG (${MATHUTILS_JAVA_PACKAGE})"
        VERBATIM)

    add_custom_target(mathutils_java_swig DEPENDS ${MATHUTILS_JAVA_WRAP_CXX})
endif()

# Whether SWIG runs here or in the driving build, the wrapper is generated, so
# mark it as such: the configure step must succeed before the file exists.
set_source_files_properties(${MATHUTILS_JAVA_WRAP_CXX} PROPERTIES GENERATED TRUE)

# android_log.cpp routes the library's log records to logcat from JNI_OnLoad;
# it is the only Android-specific file here, so it is only compiled for Android.
set(MATHUTILS_JNI_SOURCES ${MATHUTILS_JAVA_WRAP_CXX})
if(ANDROID)
    list(APPEND MATHUTILS_JNI_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/android_log.cpp)
endif()

add_library(mathutils_jni SHARED ${MATHUTILS_JNI_SOURCES})

# Linking mathutils propagates its public include dirs and C++17 requirement.
target_link_libraries(mathutils_jni PRIVATE mathutils::mathutils)

if(ANDROID)
    # The NDK sysroot already provides jni.h; -llog lets the wrapper report
    # errors via logcat if needed.
    target_link_libraries(mathutils_jni PRIVATE log)
else()
    # On a host JVM build, jni.h comes from the installed JDK.
    find_package(JNI REQUIRED)
    target_include_directories(mathutils_jni PRIVATE ${JNI_INCLUDE_DIRS})
endif()

# ---------------------------------------------------------------------------
# Optional: compile the generated Java proxies into mathutils.jar, so the
# binding can be built and used entirely from the library (host JVM), with no
# Android project involved. Off by default -- Android consumes the .java
# sources directly and has no use for a jar.
# ---------------------------------------------------------------------------
option(MATHUTILS_JAVA_BUILD_JAR "Compile the generated Java proxies into a jar" OFF)

if(MATHUTILS_JAVA_BUILD_JAR)
    find_package(Java 11 REQUIRED COMPONENTS Development)

    # Not add_jar(): the .java files do not exist at configure time, so the file
    # list is globbed by a script that runs during the build instead.
    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/mathutils.jar
        COMMAND ${CMAKE_COMMAND}
                -DJAVA_SRC_DIR=${MATHUTILS_JAVA_GENERATED_DIR}/java
                -DCLASSES_DIR=${CMAKE_BINARY_DIR}/java_classes
                -DJAR_FILE=${CMAKE_BINARY_DIR}/mathutils.jar
                -DJAVAC=${Java_JAVAC_EXECUTABLE}
                -DJAR=${Java_JAR_EXECUTABLE}
                -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/build_jar.cmake
        DEPENDS ${MATHUTILS_JAVA_WRAP_CXX}
        COMMENT "Compiling the generated Java proxies into mathutils.jar"
        VERBATIM)

    add_custom_target(mathutils_java ALL DEPENDS ${CMAKE_BINARY_DIR}/mathutils.jar)
endif()
