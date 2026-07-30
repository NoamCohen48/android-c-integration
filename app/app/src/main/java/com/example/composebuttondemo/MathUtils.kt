package com.example.composebuttondemo

import com.example.mathutils.MathNative

/**
 * Idiomatic Kotlin facade over the SWIG-generated bindings for the native
 * `mathutils` C++ library. Keeping the JNI surface behind this object means the
 * rest of the app (e.g. Compose UI) never touches the generated classes.
 *
 * The native library is loaded lazily the first time [MathNative] is used.
 */
object MathUtils {

    /** C++ `mathutils::arithmetic::add`. */
    fun add(a: Int, b: Int): Int = MathNative.add(a, b)

    /** C++ `mathutils::combinatorics::factorial` (valid for 0..20). */
    fun factorial(n: Int): Long = MathNative.factorial(n)

    /** C++ `mathutils::primes::is_prime`. */
    fun isPrime(n: Long): Boolean = MathNative.is_prime(n)

    /** C++ `mathutils::primes::primes_up_to`; returns an ordinary Kotlin list. */
    fun primesUpTo(limit: Long): List<Long> = MathNative.primes_up_to(limit).toList()
}
