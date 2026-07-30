#ifndef MATHUTILS_COMBINATORICS_FACTORIAL_HPP
#define MATHUTILS_COMBINATORICS_FACTORIAL_HPP

/// \file factorial.hpp
/// \brief Combinatorial helpers.

namespace mathutils::combinatorics {

/// Computes n! for 0 <= n <= 20.
///
/// Values above 20 overflow a signed 64-bit integer, so callers must keep
/// \p n within range.
///
/// \throws std::domain_error if \p n is negative.
[[nodiscard]] long long factorial(int n);

/// Computes the binomial coefficient "n choose k".
///
/// \throws std::domain_error if \p n or \p k is negative.
/// \returns 0 when k > n.
[[nodiscard]] long long binomial(int n, int k);

}  // namespace mathutils::combinatorics

#endif  // MATHUTILS_COMBINATORICS_FACTORIAL_HPP
