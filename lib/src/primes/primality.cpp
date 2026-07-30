#include "mathutils/primes/primality.hpp"

namespace mathutils::primes {

bool is_prime(long long n) noexcept {
    if (n < 2) {
        return false;
    }
    if (n < 4) {
        return true;  // 2 and 3 are prime.
    }
    if (n % 2 == 0 || n % 3 == 0) {
        return false;
    }

    // Check divisors of the form 6k +/- 1 up to sqrt(n).
    for (long long i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) {
            return false;
        }
    }
    return true;
}

}  // namespace mathutils::primes
