#include "mathutils/primes/sieve.hpp"

#include <cstddef>

namespace mathutils::primes {

std::vector<long long> primes_up_to(long long limit) {
    std::vector<long long> primes;
    if (limit < 2) {
        return primes;
    }

    // Sieve of Eratosthenes over [0, limit].
    std::vector<bool> is_composite(static_cast<std::size_t>(limit) + 1, false);
    for (long long i = 2; i * i <= limit; ++i) {
        if (!is_composite[static_cast<std::size_t>(i)]) {
            for (long long j = i * i; j <= limit; j += i) {
                is_composite[static_cast<std::size_t>(j)] = true;
            }
        }
    }

    for (long long i = 2; i <= limit; ++i) {
        if (!is_composite[static_cast<std::size_t>(i)]) {
            primes.push_back(i);
        }
    }
    return primes;
}

}  // namespace mathutils::primes
