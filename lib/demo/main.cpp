#include <iostream>
#include <string>

#include "mathutils/mathutils.hpp"

namespace arith = mathutils::arithmetic;
namespace comb = mathutils::combinatorics;
namespace primes = mathutils::primes;

int main() {
    std::cout << "mathutils demo (v" << mathutils::version() << ")\n";
    std::cout << "=====================\n\n";

    std::cout << "arithmetic::add(2, 3)         = " << arith::add(2, 3) << '\n';
    std::cout << "arithmetic::multiply(6, 7)    = " << arith::multiply(6, 7) << '\n';
    std::cout << '\n';

    std::cout << "combinatorics::factorial(5)   = " << comb::factorial(5) << '\n';
    std::cout << "combinatorics::factorial(20)  = " << comb::factorial(20) << '\n';
    std::cout << "combinatorics::binomial(10,3) = " << comb::binomial(10, 3) << '\n';
    std::cout << '\n';

    for (long long n : {1, 2, 17, 100, 7919}) {
        std::cout << "primes::is_prime(" << n << ")"
                  << std::string(n < 10 ? 7 : n < 100 ? 6 : n < 1000 ? 5 : 4, ' ')
                  << "= " << std::boolalpha << primes::is_prime(n) << '\n';
    }
    std::cout << '\n';

    std::cout << "primes::primes_up_to(50)      = ";
    for (long long p : primes::primes_up_to(50)) {
        std::cout << p << ' ';
    }
    std::cout << '\n';

    return 0;
}
