import org.gradle.internal.os.OperatingSystem

plugins {
    alias(libs.plugins.android.application)
    alias(libs.plugins.kotlin.android)
    alias(libs.plugins.kotlin.compose)
}

// ---------------------------------------------------------------------------
// SWIG code generation. The binding (interface, generated Java proxies and
// generated JNI wrapper) belongs to the C++ library, not to this app, so it all
// lives under lib/bindings/java/. The app only *drives* the generator, so that
// SWIG runs exactly once instead of once per ABI, and then consumes its two
// halves: the Java proxies via javac and the .cxx via the native build.
// ---------------------------------------------------------------------------
val bindingsDir = rootProject.projectDir.resolve("../lib/bindings/java")
val libIncludeDir = rootProject.projectDir.resolve("../lib/include")
val swigInterface = bindingsDir.resolve("mathutils.i")
val swigPackage = "com.example.mathutils"
val swigJavaDir = bindingsDir.resolve("generated/java")
val swigCppFile = bindingsDir.resolve("generated/cpp/mathutils_wrap.cxx")

val generateSwig = tasks.register<Exec>("generateSwig") {
    group = "build"
    description = "Generates the JNI wrapper and Java proxies from lib/bindings/java/mathutils.i via SWIG."

    inputs.file(swigInterface)
    inputs.dir(libIncludeDir)
    outputs.dir(swigJavaDir)
    outputs.file(swigCppFile)

    // Locals only: referencing script-level vals inside doFirst would capture
    // the build script, which the configuration cache cannot serialize.
    val javaOutDir = swigJavaDir.resolve(swigPackage.replace('.', '/'))
    val cppOut = swigCppFile
    doFirst {
        javaOutDir.mkdirs()
        cppOut.parentFile.mkdirs()
    }

    val swig = if (OperatingSystem.current().isWindows) "swig.exe" else "swig"
    commandLine(
        swig, "-c++", "-java",
        "-package", swigPackage,
        "-I${libIncludeDir.absolutePath}",
        "-outdir", javaOutDir.absolutePath,
        "-o", cppOut.absolutePath,
        swigInterface.absolutePath,
    )
}

android {
    namespace = "com.example.composebuttondemo"
    compileSdk = 35
    ndkVersion = "28.2.13676358"

    defaultConfig {
        applicationId = "com.example.composebuttondemo"
        minSdk = 24
        targetSdk = 35
        versionCode = 1
        versionName = "1.0"

        externalNativeBuild {
            cmake {
                // SWIG already ran once via :app:generateSwig; the per-ABI
                // native builds must not race regenerating the same files.
                arguments += "-DMATHUTILS_JAVA_RUN_SWIG=OFF"
            }
        }
        // Keep the demo lean: build for a real-device ABI and the emulator ABI.
        ndk {
            abiFilters += listOf("arm64-v8a", "x86_64")
        }
    }

    // The native build is the library's own JNI bindings project; it pulls in
    // lib/ itself, so the app passes no paths of its own.
    externalNativeBuild {
        cmake {
            path = bindingsDir.resolve("CMakeLists.txt")
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }
    kotlinOptions {
        jvmTarget = "11"
    }
    buildFeatures {
        compose = true
    }

    // Compile the library's SWIG-generated Java proxies as part of the app.
    sourceSets["main"].java.srcDir(swigJavaDir)
}

// Ensure SWIG runs before anything that consumes its output: the native build
// (needs the .cxx) and Java/Kotlin compilation (needs the proxies).
tasks.configureEach {
    val n = name
    if (n == "preBuild" ||
        n.contains("CMake") ||
        (n.startsWith("compile") && (n.contains("Kotlin") || n.contains("JavaWithJavac")))
    ) {
        dependsOn(generateSwig)
    }
}

dependencies {
    implementation(libs.androidx.core.ktx)
    implementation(libs.androidx.lifecycle.runtime.ktx)
    implementation(libs.androidx.activity.compose)
    implementation(platform(libs.androidx.compose.bom))
    implementation(libs.androidx.ui)
    implementation(libs.androidx.ui.graphics)
    implementation(libs.androidx.ui.tooling.preview)
    implementation(libs.androidx.material3)
    debugImplementation(libs.androidx.ui.tooling)
}
