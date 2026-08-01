// Minimal self-contained test harness for mathutils. Kept dependency-free so
// the example builds anywhere; a real project would use GoogleTest/Catch2.

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
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


// A PrimeObserver that records what it was handed, and can ask the scan to stop
// after a given number of primes.
class RecordingObserver : public mathutils::primes::PrimeObserver {
public:
    explicit RecordingObserver(long long stop_after = -1) : stop_after_(stop_after) {}

    bool on_prime(const mathutils::primes::PrimeEvent& event) override {
        values.push_back(event.value);
        indices.push_back(event.index);
        last_limit = event.limit;
        last_progress = event.progress;
        return stop_after_ < 0 || static_cast<long long>(values.size()) < stop_after_;
    }

    void on_finished(long long count, bool stopped_early) override {
        finished = true;
        finished_count = count;
        finished_stopped_early = stopped_early;
    }

    std::vector<long long> values;
    std::vector<long long> indices;
    long long last_limit = -1;
    double last_progress = -1.0;
    bool finished = false;
    long long finished_count = -1;
    bool finished_stopped_early = false;

private:
    long long stop_after_;
};

void test_prime_scan() {
    using namespace mathutils::primes;
    const std::vector<long long> expected = {2, 3, 5, 7, 11, 13, 17, 19};

    RecordingObserver all;
    CHECK(scan_primes(20, all) == 8);
    CHECK(all.values == expected);
    CHECK(all.indices == (std::vector<long long>{0, 1, 2, 3, 4, 5, 6, 7}));
    CHECK(all.last_limit == 20);
    CHECK(all.last_progress == 19.0 / 20.0);
    CHECK(all.finished);
    CHECK(all.finished_count == 8);
    CHECK(!all.finished_stopped_early);

    // Returning false stops the scan; the stopping prime still counts.
    RecordingObserver stopped(3);
    CHECK(scan_primes(20, stopped) == 3);
    CHECK(stopped.values == (std::vector<long long>{2, 3, 5}));
    CHECK(stopped.finished_stopped_early);

    RecordingObserver empty;
    CHECK(scan_primes(1, empty) == 0);
    CHECK(empty.values.empty());
    CHECK(empty.finished);
}

// A Sink that keeps the records, standing in for logcat / a Java sink.
class RecordingSink : public mathutils::log::Sink {
public:
    void write(mathutils::log::Level level, const std::string& tag,
               const std::string& message) override {
        levels.push_back(level);
        tags.push_back(tag);
        messages.push_back(message);
    }

    std::vector<mathutils::log::Level> levels;
    std::vector<std::string> tags;
    std::vector<std::string> messages;
};

void test_log() {
    using namespace mathutils::log;

    // No sink installed: silent, and cheap enough to skip formatting.
    CHECK(sink() == nullptr);
    CHECK(!enabled(Level::Error));

    RecordingSink recorder;
    set_sink(&recorder);
    set_min_level(Level::Info);

    CHECK(enabled(Level::Info));
    CHECK(!enabled(Level::Debug));

    write(Level::Debug, "t", "dropped");
    write(Level::Warn, "t", "kept");
    CHECK(recorder.messages == (std::vector<std::string>{"kept"}));
    CHECK(recorder.tags == (std::vector<std::string>{"t"}));

    // The library itself logs through the same sink.
    recorder.messages.clear();
    RecordingObserver observer;
    scan_primes(0, observer);
    CHECK(!recorder.messages.empty());

    CHECK(std::string(level_name(Level::Warn)) == "WARN");

    // Uninstalling stops delivery.
    const std::size_t delivered = recorder.levels.size();
    set_sink(nullptr);
    write(Level::Error, "t", "after uninstall");
    CHECK(recorder.levels.size() == delivered);
}

}  // namespace

int main() {
    test_arithmetic();
    test_combinatorics();
    test_primes();
    test_prime_scan();
    test_log();

    if (g_failures == 0) {
        std::cout << "All mathutils tests passed.\n";
        return EXIT_SUCCESS;
    }
    std::cerr << g_failures << " test check(s) failed.\n";
    return EXIT_FAILURE;
}
