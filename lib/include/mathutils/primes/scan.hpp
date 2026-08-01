#ifndef MATHUTILS_PRIMES_SCAN_HPP
#define MATHUTILS_PRIMES_SCAN_HPP

/// \file scan.hpp
/// \brief Streaming prime scan that reports each hit through a callback.
///
/// Where primes_up_to() builds the whole vector first, scan_primes() calls back
/// as it goes, handing the observer a PrimeEvent struct per prime and letting it
/// stop the scan early. That shape is what makes it a useful binding demo: the
/// callback both *receives a struct* and *returns a value* to C++.
///
/// The observer is an abstract class rather than a std::function because that is
/// what crosses language boundaries: SWIG wraps it as a director class, so a
/// Java/Kotlin object can subclass PrimeObserver and be called from C++.

namespace mathutils::primes {

/// What the scan reports for each prime it finds.
struct PrimeEvent {
    long long value = 0;    ///< The prime just found.
    long long index = 0;    ///< 0-based position in the sequence (2 -> 0).
    long long limit = 0;    ///< Upper bound the scan was started with.
    double progress = 0.0;  ///< value / limit, in [0, 1].
};

/// Callback interface for scan_primes(). Subclass it in C++ or, through the
/// SWIG binding, in Java.
class PrimeObserver {
public:
    PrimeObserver() = default;
    virtual ~PrimeObserver();

    PrimeObserver(const PrimeObserver&) = delete;
    PrimeObserver& operator=(const PrimeObserver&) = delete;

    /// Called once per prime, in ascending order.
    /// \return false to stop the scan early, true to keep going.
    virtual bool on_prime(const PrimeEvent& event) = 0;

    /// Called exactly once when the scan ends, whether it ran to \p limit or
    /// was stopped early by on_prime() returning false.
    virtual void on_finished(long long count, bool stopped_early);
};

/// Scans [2, \p limit] in ascending order, calling \p observer.on_prime() for
/// each prime, then observer.on_finished().
///
/// \return the number of primes reported (which is the number of on_prime()
///         calls, including the one that stopped the scan).
long long scan_primes(long long limit, PrimeObserver& observer);

}  // namespace mathutils::primes

#endif  // MATHUTILS_PRIMES_SCAN_HPP
