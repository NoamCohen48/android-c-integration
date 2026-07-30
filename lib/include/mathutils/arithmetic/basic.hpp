#ifndef MATHUTILS_ARITHMETIC_BASIC_HPP
#define MATHUTILS_ARITHMETIC_BASIC_HPP

/// \file basic.hpp
/// \brief Elementary arithmetic operations.

namespace mathutils::arithmetic {

/// Returns the sum of two integers.
[[nodiscard]] int add(int a, int b) noexcept;

/// Returns the product of two integers.
[[nodiscard]] int multiply(int a, int b) noexcept;

}  // namespace mathutils::arithmetic

#endif  // MATHUTILS_ARITHMETIC_BASIC_HPP
