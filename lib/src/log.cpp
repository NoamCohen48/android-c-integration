#include "mathutils/log.hpp"

#include <atomic>
#include <iostream>

namespace mathutils::log {
namespace {

// Both are read on every log call and written rarely (usually once, at
// start-up), so plain atomics are enough: no lock on the hot path, and a sink
// swapped from another thread is still published safely.
std::atomic<Sink*> g_sink{nullptr};
std::atomic<Level> g_min_level{Level::Info};

}  // namespace

Sink::~Sink() = default;

const char* level_name(Level level) {
    switch (level) {
        case Level::Verbose: return "VERBOSE";
        case Level::Debug:   return "DEBUG";
        case Level::Info:    return "INFO";
        case Level::Warn:    return "WARN";
        case Level::Error:   return "ERROR";
    }
    return "UNKNOWN";
}

void ConsoleSink::write(Level level, const std::string& tag, const std::string& message) {
    std::cerr << level_name(level) << '/' << tag << ": " << message << '\n';
}

void set_sink(Sink* sink) {
    g_sink.store(sink, std::memory_order_release);
}

Sink* sink() {
    return g_sink.load(std::memory_order_acquire);
}

void set_min_level(Level level) {
    g_min_level.store(level, std::memory_order_relaxed);
}

Level min_level() {
    return g_min_level.load(std::memory_order_relaxed);
}

bool enabled(Level level) {
    return g_sink.load(std::memory_order_acquire) != nullptr && level >= min_level();
}

void write(Level level, const std::string& tag, const std::string& message) {
    Sink* current = g_sink.load(std::memory_order_acquire);
    if (current != nullptr && level >= min_level()) {
        current->write(level, tag, message);
    }
}

}  // namespace mathutils::log
