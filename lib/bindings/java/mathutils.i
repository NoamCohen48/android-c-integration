/*
 * SWIG interface for the mathutils C++ library.
 *
 * Generates:
 *   - a JNI wrapper (mathutils_wrap.cxx / .h) compiled into libmathutils_jni.so
 *   - Java proxy classes in package com.example.mathutils
 *
 * The library headers are the single source of truth: we just %include them.
 *
 * directors="1" enables cross-language polymorphism, i.e. C++ calling *up* into
 * Java. It is what makes the callback classes below (PrimeObserver, log::Sink)
 * subclassable from Java/Kotlin; without it SWIG only generates the downward
 * (Java -> C++) direction.
 */
%module(directors="1") MathNative

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
#include "mathutils/log.hpp"
#include "mathutils/primes/primality.hpp"
#include "mathutils/primes/scan.hpp"
#include "mathutils/primes/sieve.hpp"
%}

/* std::string <-> java.lang.String (used by the log Sink). */
%include "std_string.i"

/* std::vector<long long> -> a Java LongVector (List-like) proxy. */
%include "std_vector.i"
%template(LongVector) std::vector<long long>;

/* Turn C++ exceptions (e.g. std::domain_error from factorial) into Java
 * RuntimeExceptions instead of crashing the process. */
%include "exception.i"
%exception {
    try {
        $action
    } catch (Swig::DirectorException &e) {
        /* A Java exception thrown inside a callback (see the directors below).
         * SWIG already turned the pending JVM exception into this C++ one at
         * the upcall site; re-raise the original throwable rather than
         * flattening it into a RuntimeException. */
        e.throwException(jenv);
        return $null;
    } catch (const std::exception &e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    }
}

/* ---------------------------------------------------------------------------
 * Callbacks: C++ -> Java.
 *
 * %feature("director") on an abstract C++ class makes SWIG emit a C++ subclass
 * (the "director") whose virtual methods call the Java override through JNI. A
 * Kotlin `object : PrimeObserver() { override fun on_prime(e) ... }` is then a
 * perfectly ordinary mathutils::primes::PrimeObserver as far as C++ knows.
 *
 * PrimeEvent, the struct handed to the callback, needs nothing special: SWIG
 * wraps the plain struct as a Java class with getters/setters, and the director
 * hands the Java side a proxy pointing at the C++ object for the duration of
 * the call.
 * ------------------------------------------------------------------------ */
%feature("director") mathutils::primes::PrimeObserver;
%feature("director") mathutils::log::Sink;

/* An exception thrown by a Java override is checked for right after each
 * upcall: SWIG 4.x emits that check by default (Swig::DirectorException::raise),
 * so no director:except feature is needed here — the %exception block above is
 * what turns it back into the original Java throwable at the call site. */

/* Parse the public API and generate proxies for it. */
%include "mathutils/log.hpp"
%include "mathutils/arithmetic/basic.hpp"
%include "mathutils/combinatorics/factorial.hpp"
%include "mathutils/primes/primality.hpp"
%include "mathutils/primes/scan.hpp"
%include "mathutils/primes/sieve.hpp"
