// Minimal self-contained test harness for mathutils. Kept dependency-free so
// the example builds anywhere; a real project would use GoogleTest/Catch2.

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "mathutils/mathutils.hpp"

namespace {

int g_failures = 0;

void check(bool condition, const char* expr, const char* file, int line) {
    if (!condition) {
        ++g_failures;
        std::cerr << file << ':' << line << ": FAILED: " << expr << '\n';
    }
}

#define CHECK(expr) check((expr), #expr, __FILE__, __LINE__)

void test_arithmetic() {
    using namespace mathutils::arithmetic;
    CHECK(add(2, 3) == 5);
    CHECK(add(-10, 4) == -6);
    CHECK(multiply(6, 7) == 42);
    CHECK(multiply(-3, 3) == -9);
}

void test_combinatorics() {
    using namespace mathutils::combinatorics;
    CHECK(factorial(0) == 1);
    CHECK(factorial(5) == 120);
    CHECK(factorial(20) == 2432902008176640000LL);

    CHECK(binomial(10, 0) == 1);
    CHECK(binomial(10, 3) == 120);
    CHECK(binomial(10, 10) == 1);
    CHECK(binomial(5, 6) == 0);  // k > n

    bool threw = false;
    try {
        (void)factorial(-1);
    } catch (const std::domain_error&) {
        threw = true;
    }
    CHECK(threw);
}

void test_primes() {
    using namespace mathutils::primes;
    CHECK(!is_prime(1));
    CHECK(is_prime(2));
    CHECK(is_prime(17));
    CHECK(!is_prime(100));
    CHECK(is_prime(7919));

    const std::vector<long long> expected = {2, 3, 5, 7, 11, 13, 17, 19};
    CHECK(primes_up_to(20) == expected);
    CHECK(primes_up_to(1).empty());
}

}  // namespace

int main() {
    test_arithmetic();
    test_combinatorics();
    test_primes();

    if (g_failures == 0) {
        std::cout << "All mathutils tests passed.\n";
        return EXIT_SUCCESS;
    }
    std::cerr << g_failures << " test check(s) failed.\n";
    return EXIT_FAILURE;
}
