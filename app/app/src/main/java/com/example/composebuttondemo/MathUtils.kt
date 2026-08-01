package com.example.composebuttondemo

import android.util.Log
import com.example.mathutils.Level
import com.example.mathutils.MathNative
import com.example.mathutils.PrimeEvent
import com.example.mathutils.PrimeObserver
import com.example.mathutils.Sink

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

    /** One `mathutils::primes::PrimeEvent`, copied out of C++ into Kotlin. */
    data class Prime(
        val value: Long,
        val index: Long,
        val limit: Long,
        val progress: Double,
    )

    /**
     * C++ `mathutils::primes::scan_primes`: the native code calls back *into*
     * Kotlin once per prime, handing over a `PrimeEvent` struct, and [onPrime]
     * decides whether the scan continues (`true`) or stops early (`false`).
     *
     * The callback runs on the calling thread, inside the native call — the C++
     * frame of `scan_primes` is still live underneath it. The `PrimeEvent` it
     * receives is a proxy for a C++ object that is only valid for the duration
     * of the call, which is why the fields are copied into [Prime] here.
     *
     * @return the number of primes reported.
     */
    fun scanPrimes(limit: Long, onPrime: (Prime) -> Boolean): Long {
        // A SWIG director: C++ sees a mathutils::primes::PrimeObserver whose
        // virtual calls land in this object. It only has to outlive the
        // scan_primes() call, so a local is fine (unlike the sink below).
        val observer = object : PrimeObserver() {
            override fun on_prime(event: PrimeEvent): Boolean =
                onPrime(
                    Prime(
                        value = event.value,
                        index = event.index,
                        limit = event.limit,
                        progress = event.progress,
                    )
                )

            override fun on_finished(count: Long, stopped_early: Boolean) {
                Log.d(TAG, "scan finished: count=$count stoppedEarly=$stopped_early")
            }
        }
        return MathNative.scan_primes(limit, observer)
    }

    /**
     * Routes the C++ library's log records through `android.util.Log` instead of
     * letting the native side write them to logcat itself.
     *
     * This is **not** needed to see native logs: `libmathutils_jni.so` installs
     * a logcat sink in `JNI_OnLoad` (`lib/bindings/java/android_log.cpp`), so
     * C++ logs already reach logcat under their own tags, from any thread and
     * with no JVM involvement. Use this only when the records must go somewhere
     * the JVM owns — Timber, Crashlytics, an in-app log screen.
     */
    fun routeNativeLogsToJava(enabled: Boolean = true) {
        if (!enabled) {
            MathNative.set_sink(null)
            javaSink = null
            return
        }
        val sink = object : Sink() {
            override fun write(level: Level, tag: String, message: String) {
                when (level) {
                    Level.Verbose -> Log.v(tag, message)
                    Level.Debug -> Log.d(tag, message)
                    Level.Info -> Log.i(tag, message)
                    Level.Warn -> Log.w(tag, message)
                    else -> Log.e(tag, message)
                }
            }
        }
        // A director holds only a *weak* reference to its Java peer, and C++
        // keeps this sink long after set_sink() returns — so the Java side must
        // hold a strong reference, or GC would collect it and the next native
        // log would call into nothing.
        javaSink = sink
        MathNative.set_sink(sink)
    }

    /** Lowest level the C++ library forwards to its sink. */
    fun setNativeLogLevel(level: Level) = MathNative.set_min_level(level)

    private var javaSink: Sink? = null

    private const val TAG = "MathUtils"
}
