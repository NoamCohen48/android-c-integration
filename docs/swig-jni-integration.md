# Calling the `mathutils` C++ library from Android via SWIG/JNI

This document explains how the native C++ library in `lib/` is called from the
Compose app in `app/`, so that tapping a button runs C++ (`is_prime`,
`primes_up_to`) on each tap.

## The problem being solved

The C++ lives in `lib/` as a static library (`libmathutils.a`) with functions
like `mathutils::primes::is_prime`. The Android app runs Kotlin on the ART
runtime (a JVM). The JVM cannot call C++ directly — the only bridge Android
offers is **JNI** (Java Native Interface): a JVM method declared `native` maps,
by naming convention, to a C function with a specific mangled name inside a
`.so`, which the JVM loads at runtime.

Writing that JNI glue by hand is tedious and fragile. **SWIG** generates it
automatically from the library headers. The integration therefore has two
generated halves that must agree exactly:

1. **Java side** — proxy classes with `native` method declarations.
2. **C++ side** — a wrapper `.cxx` full of `extern "C"` JNI functions that
   unpack JVM arguments, call the real C++, and repackage the result.

Both are generated from a single interface file, so they cannot drift.

## Runtime call path (what happens on a button tap)

```
Button onClick { n++ }
  -> recomposition calls MathUtils.isPrime(n)          [Kotlin facade]
    -> MathNative.is_prime(n)                           [SWIG Java proxy, static method]
      -> MathNativeJNI.is_prime(n)                      [native method declaration]
        - JVM looks up symbol Java_com_example_mathutils_MathNativeJNI_is_1prime
          in libmathutils_jni.so -
          -> that C function (in mathutils_wrap.cxx) casts jlong -> long long,
             calls mathutils::primes::is_prime(n), casts bool -> jboolean, returns
```

The first time `MathNativeJNI` is touched, its static initializer runs
`System.loadLibrary("mathutils_jni")`, loading `libmathutils_jni.so` so those
symbols exist.

## Layer by layer

### 1. The SWIG interface — `app/app/src/main/cpp/mathutils.i`

The only hand-written file describing the binding. Key parts:

- `%module MathNative` — names the generated Java classes `MathNative`
  (static-method facade) and `MathNativeJNI` (raw `native` declarations).
- The `%{ ... %}` block `#include`s the four library headers **verbatim into the
  wrapper**, so the generated C++ can see the real declarations when it compiles.
- `%include "std_vector.i"` + `%template(LongVector) std::vector<long long>;` —
  generates a Java class `LongVector extends java.util.AbstractList<Long>`, which
  is why `primes_up_to` can be treated as a normal Kotlin `List<Long>`.
- `%include "exception.i"` + the `%exception { try { $action } catch (const
  std::exception& e) { SWIG_exception(...) } }` block — wraps *every* generated
  call in a try/catch, so a C++ `std::domain_error` (e.g. `factorial(-1)`)
  becomes a Java `RuntimeException` instead of crashing the process.
- `%pragma(java) jniclasscode=%{ static { System.loadLibrary("mathutils_jni"); }
  %}` — injects the library-loading static block into `MathNativeJNI` (SWIG does
  not add this by default).
- The four `%include "mathutils/.../*.hpp"` lines — this is what makes SWIG
  *parse the API and generate proxies*. (The earlier `#include` inside `%{ %}`
  only makes declarations available to the C++ compiler; `%include` is what SWIG
  introspects.) SWIG 4.4 parses the C++17 nested namespaces, `[[nodiscard]]`, and
  `noexcept` fine, and flattens the free functions from
  `mathutils::arithmetic` / `combinatorics` / `primes` into static methods on
  `MathNative` (there are no name collisions among the six functions).

### 2. What SWIG emits

Running `swig -c++ -java -package com.example.mathutils` produces:

- `MathNativeJNI.java` — `public final static native boolean is_prime(long)`,
  etc., plus the `loadLibrary` static block.
- `MathNative.java` — friendly `public static boolean is_prime(long n)` wrappers
  that call `MathNativeJNI`.
- `LongVector.java` — the `List<Long>` proxy.
- `mathutils_wrap.cxx` — ~865 lines of `extern "C"` functions named
  `Java_com_example_mathutils_MathNativeJNI_<method>` (the `1` in `is_1prime` is
  JNI's escaping of the underscore). Each converts JVM types to/from C++ types
  and calls the library.

The JNI naming convention is the contract: package `com.example.mathutils` +
class `MathNativeJNI` + method -> exact C symbol name. If any of those change,
the symbol will not resolve at runtime.

### 3. The Kotlin facade — `MathUtils.kt`

The generated classes are awkward (`MathNative.is_prime`, a `LongVector`). The
facade is a thin `object MathUtils` exposing `isPrime(n)`,
`primesUpTo(n).toList()`, etc. The rest of the app — all the Compose code —
imports **only** `MathUtils`, never the generated JNI classes. If SWIG is ever
replaced, only this one file changes.

### 4. The UI — `MainActivity.kt`

`CounterScreen` holds `n`; each tap recomposes and calls `MathUtils.isPrime(n)`
and `MathUtils.primesUpTo(n)`, so the displayed values are computed in C++. The
`LocalInspectionMode.current` guard skips the native calls in Android Studio's
preview renderer (which has no `.so` and would throw `UnsatisfiedLinkError`).

## Build orchestration

The two generated halves are consumed by two different compilers (Java and C++),
and both must wait for SWIG. It is wired so SWIG runs exactly **once**.

### The `generateSwig` Gradle task (`app/app/build.gradle.kts`)

An `Exec` task that runs the `swig` command, writing:

- Java proxies -> `app/app/build/generated/swig/java/com/example/mathutils/`
- C++ wrapper -> `app/app/build/generated/swig/cpp/mathutils_wrap.cxx`

It declares `inputs` (the `.i` file, the lib headers) and `outputs`, so Gradle
caches it and only reruns when those change.

### Feeding the Java compiler

`sourceSets["main"].java.srcDir(swigJavaDir)` adds the generated Java folder to
the module's sources, so `MathNative` / `LongVector` compile alongside the Kotlin.

### Feeding the C++ compiler — `externalNativeBuild` + `CMakeLists.txt`

`android.externalNativeBuild.cmake.path` points at `src/main/cpp/CMakeLists.txt`,
which:

- receives two absolute paths from Gradle via `cmake.arguments`:
  `-DMATHUTILS_LIB_DIR` (the `lib/` folder) and `-DSWIG_WRAP_CXX` (the generated
  wrapper).
- `add_subdirectory(${MATHUTILS_LIB_DIR} ...)` pulls the library in **unchanged**,
  with `MATHUTILS_BUILD_DEMO/TESTS=OFF` (the lib's `install()` rules are guarded
  by `PROJECT_IS_TOP_LEVEL`, so they stay off here too).
- `set_source_files_properties(... GENERATED TRUE)` tells CMake the wrapper is
  produced externally, so the *configure* step does not fail when the file does
  not yet exist.
- `add_library(mathutils_jni SHARED ${SWIG_WRAP_CXX})` +
  `target_link_libraries(... mathutils::mathutils log)` produces the `.so`.
  Linking `mathutils::mathutils` propagates its public include dirs and its
  `cxx_std_17` requirement to the wrapper automatically.

### Ordering

Both consumers must wait for `generateSwig`. A `tasks.configureEach` block
enforces it:

```kotlin
if (n == "preBuild" || n.contains("CMake") ||
    (n.startsWith("compile") && (n.contains("Kotlin") || n.contains("JavaWithJavac"))))
    dependsOn(generateSwig)
```

So `configureCMakeDebug[abi]`, `buildCMakeDebug[abi]`, `compileDebugKotlin`, and
`compileDebugJavaWithJavac` all depend on SWIG having run.

### ABIs

`ndk { abiFilters += listOf("arm64-v8a", "x86_64") }` builds the `.so` twice —
once per CPU architecture (real devices vs. emulator). The native build runs
per-ABI, but SWIG runs only once because it is decoupled into the Gradle task,
not driven by CMake — so there is no race regenerating the Java from parallel
ABI builds.

### Toolchain pointers

- `ndkVersion = "28.2.13676358"` pins the NDK (compiler + `jni.h` + libc++).
- `cmake.dir=/usr` in `local.properties` points AGP at the system CMake/Ninja,
  since there is no SDK-bundled CMake on this machine. It is git-ignored and
  machine-local; a clone on a machine with an SDK CMake needs no change.

## How the app knows *where* and *how* to compile the C++ library

Two separate mechanisms answer these two questions: **where** the C++ source is,
and **how** it gets compiled.

### "Where" — a two-hop pointer chain

The app never hardcodes the C++ `.cpp` files. It finds them through two
indirections:

**Hop 1 — Gradle tells AGP which CMake project to build.** In
`app/app/build.gradle.kts`:

```kotlin
android {
    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")   // the entry point
        }
    }
}
```

`externalNativeBuild.cmake.path` is the one thing connecting the Gradle world to
the CMake world.

**Hop 2 — that CMakeLists is told where `lib/` is, via a variable.** The app's
CMakeLists does not contain a path to `lib/`; Gradle passes it in as a `-D`
argument:

```kotlin
val libRootDir = rootProject.projectDir.resolve("../lib")   // android-c/app + ../lib = android-c/lib
...
defaultConfig {
    externalNativeBuild {
        cmake {
            arguments += listOf(
                "-DMATHUTILS_LIB_DIR=${libRootDir.absolutePath}",
                "-DSWIG_WRAP_CXX=${swigCppFile.get().asFile.absolutePath}",
            )
        }
    }
}
```

`rootProject.projectDir` is `android-c/app` (the Gradle root), so `../lib`
resolves to `android-c/lib`, made absolute. That value arrives in CMake as
`${MATHUTILS_LIB_DIR}`.

**Hop 3 — CMake pulls the library in and reads *its* file list.** In
`app/app/src/main/cpp/CMakeLists.txt`:

```cmake
add_subdirectory(${MATHUTILS_LIB_DIR} ${CMAKE_BINARY_DIR}/mathutils)
```

`add_subdirectory` runs `lib/`'s **own** CMake, which is what actually lists the
sources (`lib/src/CMakeLists.txt`):

```cmake
add_library(mathutils
    arithmetic/basic.cpp
    combinatorics/factorial.cpp
    primes/primality.cpp
    primes/sieve.cpp)
```

So the authoritative source list lives with the library, not the app. The app
only knows an absolute path to the library root; the library describes itself.
The second argument to `add_subdirectory` (`${CMAKE_BINARY_DIR}/mathutils`) is
required because `lib/` sits *outside* the app's source tree, so CMake needs a
place to put its build outputs.

```
build.gradle.kts  --path-->  app/.../cpp/CMakeLists.txt  --DMATHUTILS_LIB_DIR-->  lib/CMakeLists.txt  --lists-->  *.cpp
```

### "How" — AGP drives CMake with the NDK toolchain, once per ABI

"How to compile" is handled by AGP + the NDK. When `assembleDebug` runs, AGP
generates a CMake invocation for **each** ABI and runs it, supplying:

- **Which CMake binary** — from `cmake.dir=/usr` in `local.properties`, with
  Ninja found alongside it; AGP calls CMake with `-GNinja`.
- **Which compiler / sysroot** — from `ndkVersion = "28.2.13676358"`. AGP passes
  `-DCMAKE_TOOLCHAIN_FILE=<ndk>/build/cmake/android.toolchain.cmake`, which makes
  CMake use the NDK's Clang, Android's libc++, and the right headers (`jni.h`
  comes from here) instead of the host `g++`.
- **Which architecture** — `-DANDROID_ABI=arm64-v8a` (then `x86_64`), from
  `ndk { abiFilters = [...] }`. This is why `configureCMakeDebug[arm64-v8a]` and
  `configureCMakeDebug[x86_64]` appear as separate tasks.
- **Which API level** — `-DANDROID_PLATFORM=android-24`, from `minSdk = 24`.

Each ABI gets its own configure + build, compiling `lib/`'s `.cpp` files into a
per-ABI `libmathutils.a`, then linking them into `libmathutils_jni.so` for that
ABI (via `target_link_libraries(mathutils_jni PRIVATE mathutils::mathutils log)`).

The **compile options** come from the library's own CMake, not the app:

- `target_compile_features(mathutils PUBLIC cxx_std_17)` compiles each unit as
  C++17; being `PUBLIC`, that requirement also propagates to the SWIG wrapper
  that links it.
- the `mathutils::warnings` interface target adds `-Wall -Wextra -Wpedantic ...`
  (applied `PRIVATE`, so the app is not forced to inherit them).
- `target_include_directories(... PUBLIC .../include ...)` propagates to the
  wrapper automatically when it links `mathutils::mathutils`, which is how
  `mathutils_wrap.cxx`'s `#include "mathutils/primes/primality.hpp"` resolves.

### After compilation

The resulting `.so` files (one per ABI) are emitted under
`app/app/build/intermediates/...`, then AGP's `mergeDebugNativeLibs` ->
`stripDebugDebugSymbols` -> `packageDebug` collect and drop them into the APK at
`lib/arm64-v8a/` and `lib/x86_64/`.

In short: Gradle knows **where** through the `cmake.path` -> `-DMATHUTILS_LIB_DIR`
-> `add_subdirectory` chain, and **how** through the NDK toolchain file, ABI, and
platform flags AGP feeds to CMake — while the actual source list and compile
flags stay owned by `lib/`'s own CMake.

## How the pieces line up (the invariants)

The integration only works because these agree:

| Thing         | Value                   | Set in                                                                              |
| ------------- | ----------------------- | ----------------------------------------------------------------------------------- |
| Java package  | `com.example.mathutils` | `-package` flag **and** the output folder path                                      |
| JNI class     | `MathNativeJNI`         | `%module MathNative`                                                                 |
| Library name  | `mathutils_jni`         | `System.loadLibrary(...)` **and** `add_library(mathutils_jni ...)` -> `libmathutils_jni.so` |

Break any one and you get a compile error or a runtime `UnsatisfiedLinkError` —
which is why generating both sides from one `.i` is the elegant part: they
cannot drift.

## Files involved

| File                                                          | Purpose                                                          |
| ------------------------------------------------------------- | --------------------------------------------------------------- |
| `app/app/src/main/cpp/mathutils.i`                            | SWIG interface (the only hand-written binding description)       |
| `app/app/src/main/cpp/CMakeLists.txt`                         | Builds `libmathutils_jni.so` from the wrapper + `lib/`           |
| `app/app/build.gradle.kts`                                    | `generateSwig` task, NDK/CMake config, source set, task ordering |
| `app/app/src/main/java/com/example/composebuttondemo/MathUtils.kt` | Idiomatic Kotlin facade over the SWIG proxies              |
| `app/app/src/main/java/com/example/composebuttondemo/MainActivity.kt` | Button runs `is_prime` / `primes_up_to` in C++         |
| `app/local.properties`                                        | `cmake.dir=/usr` (git-ignored, machine-local)                   |

## Building and verifying

```sh
cd app
JAVA_HOME=/usr/lib/jvm/java-21-openjdk ANDROID_HOME=/home/noamc/Android/Sdk \
  ./gradlew :app:assembleDebug
```

The resulting APK (`app/app/build/outputs/apk/debug/app-debug.apk`) contains:

- `lib/arm64-v8a/libmathutils_jni.so` and `lib/x86_64/libmathutils_jni.so`
- dex classes `MathNative`, `MathNativeJNI`, `LongVector`, and `MathUtils`

confirming both generated halves built and were packaged.

## Regenerating manually (for inspection)

To see exactly what SWIG produces, without Gradle:

```sh
swig -c++ -java \
  -package com.example.mathutils \
  -I lib/include \
  -outdir /tmp/swig/java/com/example/mathutils \
  -o /tmp/swig/mathutils_wrap.cxx \
  app/app/src/main/cpp/mathutils.i
```
