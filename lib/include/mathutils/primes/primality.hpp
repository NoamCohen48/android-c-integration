#ifndef MATHUTILS_PRIMES_PRIMALITY_HPP
#define MATHUTILS_PRIMES_PRIMALITY_HPP

/// \file primality.hpp
/// \brief Primality testing.

namespace mathutils::primes {

/// Returns true if \p n is a prime number.
///
/// Values less than 2 are not prime. Uses 6k +/- 1 trial division, which is
/// exact for the full 64-bit range (though slow for very large inputs).
[[nodiscard]] bool is_prime(long long n) noexcept;

}  // namespace mathutils::primes

#endif  // MATHUTILS_PRIMES_PRIMALITY_HPP
