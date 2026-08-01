/*
 * SWIG interface for the mathutils C++ library.
 *
 * Generates:
 *   - a JNI wrapper (mathutils_wrap.cxx) compiled into libmathutils_jni.so
 *   - Java proxy classes in package com.example.mathutils
 *
 * The library headers are the single source of truth: we just %include them.
 */
%module MathNative

/* Load the native library when the generated JNI class is first used. */
%pragma(java) jniclasscode=%{
    static {
        System.loadLibrary("mathutils_jni");
    }
%}

%{
/* Emitted verbatim into the wrapper; makes the real declarations available. */
#include "mathutils/arithmetic/basic.hpp"
#include "mathutils/combinatorics/factorial.hpp"
#include "mathutils/primes/primality.hpp"
#include "mathutils/primes/sieve.hpp"
%}

/* std::vector<long long> -> a Java LongVector (List-like) proxy. */
%include "std_vector.i"
%template(LongVector) std::vector<long long>;

/* Turn C++ exceptions (e.g. std::domain_error from factorial) into Java
 * RuntimeExceptions instead of crashing the process. */
%include "exception.i"
%exception {
    try {
        $action
    } catch (const std::exception &e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    }
}

/* Parse the public API and generate proxies for it. */
%include "mathutils/arithmetic/basic.hpp"
%include "mathutils/combinatorics/factorial.hpp"
%include "mathutils/primes/primality.hpp"
%include "mathutils/primes/sieve.hpp"
