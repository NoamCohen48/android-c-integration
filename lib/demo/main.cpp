#include <iostream>
#include <string>

#include "mathutils/mathutils.hpp"

namespace arith = mathutils::arithmetic;
namespace comb = mathutils::combinatorics;
namespace mlog = mathutils::log;  // not `log`: clashes with ::log(double)
namespace primes = mathutils::primes;

namespace {

/// Callback for scan_primes(): prints the struct it is handed and stops the
/// scan once it has seen enough primes. The Java binding does the same thing by
/// subclassing PrimeObserver from Kotlin (see the Android app).
class PrintingObserver : public primes::PrimeObserver {
public:
    explicit PrintingObserver(long long stop_after) : stop_after_(stop_after) {}

    bool on_prime(const primes::PrimeEvent& event) override {
        std::cout << "  on_prime{value=" << event.value << ", index=" << event.index
                  << ", limit=" << event.limit << ", progress=" << event.progress << "}\n";
        return event.index + 1 < stop_after_;
    }

    void on_finished(long long count, bool stopped_early) override {
        std::cout << "  on_finished{count=" << count
                  << ", stopped_early=" << std::boolalpha << stopped_early << "}\n";
    }

private:
    long long stop_after_;
};

}  // namespace

int main() {
    // Send the library's log records somewhere. On Android the JNI bindings
    // install a sink that forwards to logcat instead; the library itself never
    // decides where logs go.
    mlog::ConsoleSink console;
    mlog::set_sink(&console);
    mlog::set_min_level(mlog::Level::Debug);

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

    std::cout << "primes::scan_primes(50, observer) — callback per prime:\n";
    PrintingObserver observer(5);
    const long long reported = primes::scan_primes(50, observer);
    std::cout << "  -> scan_primes returned " << reported << '\n';

    mlog::set_sink(nullptr);
    return 0;
}
