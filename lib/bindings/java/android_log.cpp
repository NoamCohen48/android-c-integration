/*
 * Android-only adapter: routes the library's log records to logcat.
 *
 * The library itself has no idea Android exists -- it writes to an abstract
 * mathutils::log::Sink (see lib/include/mathutils/log.hpp). This file is the
 * one place that knows about `android/log.h`, and it is compiled into
 * libmathutils_jni.so only when building for Android.
 *
 * It is installed from JNI_OnLoad, i.e. the moment System.loadLibrary() maps
 * the library, so every record the C++ produces afterwards -- including from
 * native threads with no Java on the stack -- reaches logcat directly, with no
 * JNI round-trip and nothing to set up from Kotlin.
 *
 * A Java sink is still possible (mathutils::log::Sink is a SWIG director, so
 * Kotlin can subclass it and MathNative.set_sink() it); that just replaces this
 * one. Prefer this default: __android_log_write is the documented NDK path to
 * logcat, it is thread-safe, and it does not need a JNIEnv.
 */
#include <jni.h>

#include <android/log.h>

#include "mathutils/log.hpp"

namespace {

android_LogPriority to_android_priority(mathutils::log::Level level) {
    switch (level) {
        case mathutils::log::Level::Verbose: return ANDROID_LOG_VERBOSE;
        case mathutils::log::Level::Debug:   return ANDROID_LOG_DEBUG;
        case mathutils::log::Level::Info:    return ANDROID_LOG_INFO;
        case mathutils::log::Level::Warn:    return ANDROID_LOG_WARN;
        case mathutils::log::Level::Error:   return ANDROID_LOG_ERROR;
    }
    return ANDROID_LOG_INFO;
}

class LogcatSink : public mathutils::log::Sink {
public:
    void write(mathutils::log::Level level, const std::string& tag,
               const std::string& message) override {
        // __android_log_write takes the message as-is (no printf formatting),
        // which is what we want for an already-formatted string, and is safe to
        // call from any thread.
        __android_log_write(to_android_priority(level), tag.c_str(), message.c_str());
    }
};

// Static storage duration: the sink outlives every log call, and the library
// only ever borrows the pointer.
LogcatSink g_logcat_sink;

}  // namespace

/// Called by the JVM when libmathutils_jni.so is loaded. SWIG does not generate
/// one, so this is ours to define.
extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* /*vm*/, void* /*reserved*/) {
    mathutils::log::set_sink(&g_logcat_sink);
    mathutils::log::set_min_level(mathutils::log::Level::Debug);
    __android_log_write(ANDROID_LOG_INFO, "mathutils",
                        "native logging installed (JNI_OnLoad)");
    return JNI_VERSION_1_6;
}
