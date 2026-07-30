import org.gradle.internal.os.OperatingSystem

plugins {
    alias(libs.plugins.android.application)
    alias(libs.plugins.kotlin.android)
    alias(libs.plugins.kotlin.compose)
}

// ---------------------------------------------------------------------------
// SWIG code generation: turns lib/ headers into a JNI wrapper (compiled into
// libmathutils_jni.so by CMake) plus Java proxy classes in package
// com.example.mathutils. Runs once, feeding both the native build and javac.
// ---------------------------------------------------------------------------
val swigInterface = layout.projectDirectory.file("src/main/cpp/mathutils.i")
val libIncludeDir = rootProject.projectDir.resolve("../lib/include")
val libRootDir = rootProject.projectDir.resolve("../lib")
val swigJavaDir = layout.buildDirectory.dir("generated/swig/java")
val swigCppFile = layout.buildDirectory.file("generated/swig/cpp/mathutils_wrap.cxx")

val generateSwig = tasks.register<Exec>("generateSwig") {
    group = "build"
    description = "Generates the JNI wrapper and Java proxies from mathutils.i via SWIG."

    inputs.file(swigInterface)
    inputs.dir(libIncludeDir)
    outputs.dir(swigJavaDir)
    outputs.file(swigCppFile)

    val javaOutDir = swigJavaDir.get().dir("com/example/mathutils").asFile
    val cppOut = swigCppFile.get().asFile
    doFirst {
        javaOutDir.mkdirs()
        cppOut.parentFile.mkdirs()
    }

    val swig = if (OperatingSystem.current().isWindows) "swig.exe" else "swig"
    commandLine(
        swig, "-c++", "-java",
        "-package", "com.example.mathutils",
        "-I${libIncludeDir.absolutePath}",
        "-outdir", javaOutDir.absolutePath,
        "-o", cppOut.absolutePath,
        swigInterface.asFile.absolutePath,
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
                arguments += listOf(
                    "-DMATHUTILS_LIB_DIR=${libRootDir.absolutePath}",
                    "-DSWIG_WRAP_CXX=${swigCppFile.get().asFile.absolutePath}",
                )
            }
        }
        // Keep the demo lean: build for a real-device ABI and the emulator ABI.
        ndk {
            abiFilters += listOf("arm64-v8a", "x86_64")
        }
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
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

    // Compile the SWIG-generated Java proxies as part of the app.
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
