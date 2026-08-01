#include "mathutils/primes/scan.hpp"

#include <sstream>
#include <vector>

#include "mathutils/log.hpp"

namespace mathutils::primes {
namespace {

constexpr const char* kTag = "mathutils.scan";

}  // namespace

PrimeObserver::~PrimeObserver() = default;

void PrimeObserver::on_finished(long long /*count*/, bool /*stopped_early*/) {}

long long scan_primes(long long limit, PrimeObserver& observer) {
    MATHUTILS_LOG(Debug, kTag, "scan_primes(limit=" << limit << ") starting");

    if (limit < 2) {
        MATHUTILS_LOG(Warn, kTag, "limit " << limit << " is below 2; nothing to scan");
        observer.on_finished(0, false);
        return 0;
    }

    // Same sieve as primes_up_to(), but the results are streamed to the
    // observer instead of collected, so a caller can stop at any point.
    std::vector<bool> is_composite(static_cast<std::size_t>(limit) + 1, false);
    for (long long i = 2; i * i <= limit; ++i) {
        if (!is_composite[static_cast<std::size_t>(i)]) {
            for (long long j = i * i; j <= limit; j += i) {
                is_composite[static_cast<std::size_t>(j)] = true;
            }
        }
    }

    long long count = 0;
    bool stopped_early = false;
    for (long long n = 2; n <= limit; ++n) {
        if (is_composite[static_cast<std::size_t>(n)]) {
            continue;
        }

        PrimeEvent event;
        event.value = n;
        event.index = count;
        event.limit = limit;
        event.progress = static_cast<double>(n) / static_cast<double>(limit);
        ++count;

        if (!observer.on_prime(event)) {
            stopped_early = true;
            MATHUTILS_LOG(Info, kTag, "observer stopped the scan at " << n);
            break;
        }
    }

    MATHUTILS_LOG(Info, kTag,
                  "scan_primes(limit=" << limit << ") reported " << count
                                       << (stopped_early ? " prime(s), stopped early"
                                                         : " prime(s)"));
    observer.on_finished(count, stopped_early);
    return count;
}

}  // namespace mathutils::primes
