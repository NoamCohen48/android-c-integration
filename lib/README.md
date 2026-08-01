# mathutils

A small C++17 math utilities library, organised the way a larger library would
be: each area of functionality is its own **component** with a matching
subfolder in `include/` and `src/`, its own nested namespace, and its own
translation unit.

## Layout

```
lib/
├── CMakeLists.txt                       # top-level: options, warnings, install/export
├── cmake/
│   └── mathutilsConfig.cmake.in         # find_package(mathutils) package config
├── include/mathutils/                   # public headers (installed)
│   ├── mathutils.hpp                    # umbrella header (includes everything)
│   ├── version.hpp.in                   # -> generated version.hpp
│   ├── log.hpp                          # namespace mathutils::log (Sink facade)
│   ├── arithmetic/basic.hpp             # namespace mathutils::arithmetic
│   ├── combinatorics/factorial.hpp      # namespace mathutils::combinatorics
│   └── primes/
│       ├── primality.hpp                # namespace mathutils::primes
│       ├── scan.hpp                     # callback-based prime scan
│       └── sieve.hpp
├── src/                                 # implementation, mirrors include/ layout
│   ├── CMakeLists.txt                   # the `mathutils` library target
│   ├── arithmetic/basic.cpp
│   ├── combinatorics/factorial.cpp
│   ├── log.cpp
│   └── primes/{primality,scan,sieve}.cpp
├── bindings/java/                       # SWIG Java/JNI binding (see its README)
│   ├── CMakeLists.txt                   # libmathutils_jni + SWIG generation
│   └── mathutils.i                      # SWIG interface (generated/ is ignored)
├── demo/                                # mathutils_demo executable
│   ├── CMakeLists.txt
│   └── main.cpp
└── tests/                               # mathutils_tests, registered with CTest
    ├── CMakeLists.txt
    └── mathutils_tests.cpp
```

## Components

| Namespace                  | Header                            | Provides                          |
| -------------------------- | --------------------------------- | --------------------------------- |
| `mathutils::arithmetic`    | `mathutils/arithmetic/basic.hpp`  | `add`, `multiply`                 |
| `mathutils::combinatorics` | `mathutils/combinatorics/factorial.hpp` | `factorial`, `binomial`     |
| `mathutils::primes`        | `mathutils/primes/primality.hpp`  | `is_prime`                        |
| `mathutils::primes`        | `mathutils/primes/sieve.hpp`      | `primes_up_to`                    |
| `mathutils::primes`        | `mathutils/primes/scan.hpp`       | `scan_primes`, `PrimeObserver`, `PrimeEvent` |
| `mathutils::log`           | `mathutils/log.hpp`               | `Sink`, `ConsoleSink`, `set_sink`, `set_min_level` |

## Build & run

```sh
cmake -S lib -B lib/build
cmake --build lib/build
./lib/build/demo/mathutils_demo
```

Run the tests:

```sh
ctest --test-dir lib/build --output-on-failure
```

`lib/build/` is generated output and is not meant to be committed
(`rm -rf lib/build` to clean).

### CMake options

| Option                 | Default | Effect                        |
| ---------------------- | ------- | ----------------------------- |
| `MATHUTILS_BUILD_DEMO` | `ON`    | Build the `mathutils_demo`    |
| `MATHUTILS_BUILD_TESTS`| `ON`    | Build the CTest test suite    |

## Using it from another CMake project

After `cmake --install lib/build --prefix <somewhere>`:

```cmake
find_package(mathutils REQUIRED)
target_link_libraries(my_app PRIVATE mathutils::mathutils)
```

Or vendor it directly:

```cmake
add_subdirectory(lib)
target_link_libraries(my_app PRIVATE mathutils::mathutils)
```

## Using it from Java / Android

`bindings/java/` holds the SWIG interface that turns the public headers into
Java proxies plus a JNI wrapper (`libmathutils_jni`). It builds standalone:

```sh
cmake -S lib/bindings/java -B lib/bindings/java/build -DMATHUTILS_JAVA_BUILD_JAR=ON
cmake --build lib/bindings/java/build
```

The Android app in `app/` consumes that same binding project rather than
carrying its own copy. See `lib/bindings/java/README.md`.

## Callbacks (C++ calling you back)

`primes/scan.hpp` streams results instead of returning them: `scan_primes()`
hands a `PrimeEvent` struct to a `PrimeObserver` for each prime and stops early
when the observer returns `false`.

```cpp
class Printer : public mathutils::primes::PrimeObserver {
    bool on_prime(const mathutils::primes::PrimeEvent& e) override {
        std::cout << e.index << ": " << e.value << " (" << e.progress << ")\n";
        return e.index < 4;              // false stops the scan
    }
};

Printer printer;
mathutils::primes::scan_primes(50, printer);
```

The observer is an abstract class, not a `std::function`, because that is what
crosses a language boundary: SWIG wraps it as a *director*, so Java/Kotlin can
subclass it and be called from C++ (see `lib/bindings/java/`).

## Logging

The library never decides where its logs go. It writes records to an abstract
`mathutils::log::Sink` that the host installs once:

```cpp
mathutils::log::ConsoleSink console;          // or your own Sink subclass
mathutils::log::set_sink(&console);
mathutils::log::set_min_level(mathutils::log::Level::Debug);
```

With no sink installed the library is silent, and disabled levels cost nothing —
`MATHUTILS_LOG` does not format the message. `Sink::write` may be called from
any thread, so implementations must be thread-safe.

On Android the JNI bindings install a sink that forwards to **logcat** from
`JNI_OnLoad` (`lib/bindings/java/android_log.cpp`), so native logs appear in
Logcat with no Java setup; a Kotlin `Sink` subclass can replace it. Only that one
file knows Android exists.
