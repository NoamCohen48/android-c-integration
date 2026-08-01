#ifndef MATHUTILS_LOG_HPP
#define MATHUTILS_LOG_HPP

/// \file log.hpp
/// \brief Minimal logging facade: the library emits records, the host decides
///        where they go.
///
/// The library must not know about Android, so it never calls
/// `__android_log_print` itself. Instead it writes to an abstract Sink that the
/// host installs once at start-up:
///
///   - on Android, the JNI bindings install a sink that forwards to logcat (see
///     `lib/bindings/java/android_log.cpp`), so C++ logs show up in Logcat with
///     no Java code involved and from any thread;
///   - on a host build, install ConsoleSink (or your own) to get the records on
///     stderr;
///   - a Java/Kotlin sink can be installed too: the SWIG binding exposes Sink as
///     a director class, so Java can subclass it and receive the records.
///
/// No sink is installed by default: unhosted, the library is silent.

#include <string>

namespace mathutils::log {

/// Severity of a log record, mirroring the levels every host logger has.
enum class Level {
    Verbose = 0,
    Debug = 1,
    Info = 2,
    Warn = 3,
    Error = 4,
};

/// Human-readable name of \p level ("DEBUG", "WARN", ...).
[[nodiscard]] const char* level_name(Level level);

/// Destination for log records. Subclass it (in C++ or, via the SWIG director,
/// in Java) and install it with set_sink().
///
/// write() may be called from any thread, so implementations must be
/// thread-safe.
class Sink {
public:
    Sink() = default;
    virtual ~Sink();

    Sink(const Sink&) = delete;
    Sink& operator=(const Sink&) = delete;

    virtual void write(Level level, const std::string& tag, const std::string& message) = 0;
};

/// A ready-made Sink that prints "LEVEL/tag: message" lines to stderr.
class ConsoleSink : public Sink {
public:
    void write(Level level, const std::string& tag, const std::string& message) override;
};

/// Installs \p sink as the destination for all subsequent records, replacing
/// any previous one. Ownership stays with the caller: the sink must outlive the
/// last log call. Pass nullptr to silence the library.
void set_sink(Sink* sink);

/// The currently installed sink, or nullptr.
[[nodiscard]] Sink* sink();

/// The minimum level that is forwarded to the sink (default: Info). Records
/// below it are dropped cheaply, before the message is even formatted.
void set_min_level(Level level);
[[nodiscard]] Level min_level();

/// Emits one record. Library code normally calls this through MATHUTILS_LOG.
void write(Level level, const std::string& tag, const std::string& message);

/// True when a record at \p level would be forwarded; lets callers skip
/// building an expensive message.
[[nodiscard]] bool enabled(Level level);

}  // namespace mathutils::log

/// Internal convenience: MATHUTILS_LOG(Info, "tag", "x=" << x). The stream
/// expression is only evaluated when the level is enabled.
#define MATHUTILS_LOG(level_, tag_, expr_)                                       \
    do {                                                                         \
        if (::mathutils::log::enabled(::mathutils::log::Level::level_)) {         \
            ::std::ostringstream mathutils_log_oss_;                              \
            mathutils_log_oss_ << expr_;                                          \
            ::mathutils::log::write(::mathutils::log::Level::level_, (tag_),      \
                                    mathutils_log_oss_.str());                    \
        }                                                                        \
    } while (false)

#endif  // MATHUTILS_LOG_HPP
