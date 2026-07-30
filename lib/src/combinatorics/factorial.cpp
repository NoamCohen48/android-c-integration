#include "mathutils/combinatorics/factorial.hpp"

#include <stdexcept>

namespace mathutils::combinatorics {

long long factorial(int n) {
    if (n < 0) {
        throw std::domain_error("factorial: n must be non-negative");
    }

    long long result = 1;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}

long long binomial(int n, int k) {
    if (n < 0 || k < 0) {
        throw std::domain_error("binomial: n and k must be non-negative");
    }
    if (k > n) {
        return 0;
    }

    // Use the smaller symmetric term and multiply iteratively to limit the
    // magnitude of intermediate values.
    if (k > n - k) {
        k = n - k;
    }

    long long result = 1;
    for (int i = 1; i <= k; ++i) {
        result = result * (n - k + i) / i;
    }
    return result;
}

}  // namespace mathutils::combinatorics
