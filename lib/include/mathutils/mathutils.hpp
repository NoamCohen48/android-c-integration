#ifndef MATHUTILS_MATHUTILS_HPP
#define MATHUTILS_MATHUTILS_HPP

/// \file mathutils.hpp
/// \brief Umbrella header exposing the complete mathutils public API.
///
/// Prefer including the individual component headers (e.g.
/// `<mathutils/primes/sieve.hpp>`) in translation units that only need part of
/// the library. This convenience header is provided for demos and quick use.
///
/// The API is organised into components, each in its own nested namespace:
///   - mathutils::arithmetic    — elementary arithmetic
///   - mathutils::combinatorics — factorials and binomials
///   - mathutils::primes        — primality testing, generation and scanning
///   - mathutils::log           — where the library's log records go

#include "mathutils/version.hpp"

#include "mathutils/arithmetic/basic.hpp"
#include "mathutils/combinatorics/factorial.hpp"
#include "mathutils/log.hpp"
#include "mathutils/primes/primality.hpp"
#include "mathutils/primes/scan.hpp"
#include "mathutils/primes/sieve.hpp"

#endif  // MATHUTILS_MATHUTILS_HPP
