#ifndef MATHUTILS_PRIMES_SIEVE_HPP
#define MATHUTILS_PRIMES_SIEVE_HPP

/// \file sieve.hpp
/// \brief Prime generation via the Sieve of Eratosthenes.

#include <vector>

namespace mathutils::primes {

/// Returns all primes p with 2 <= p <= \p limit, in ascending order.
///
/// An empty vector is returned when \p limit < 2.
[[nodiscard]] std::vector<long long> primes_up_to(long long limit);

}  // namespace mathutils::primes

#endif  // MATHUTILS_PRIMES_SIEVE_HPP
