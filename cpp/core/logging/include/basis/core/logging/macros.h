#pragma once

#include <basis/core/logging.h>

#include <spdlog/spdlog.h>
#include <string>
#include <string_view>

// Convenience macros to do "the right thing" given an appropriately scoped logger
#define BASIS_LOG_TRACE(...) SPDLOG_LOGGER_TRACE(AUTO_LOGGER, __VA_ARGS__)
#define BASIS_LOG_DEBUG(...) SPDLOG_LOGGER_DEBUG(AUTO_LOGGER, __VA_ARGS__)
#define BASIS_LOG_INFO(...) SPDLOG_LOGGER_INFO(AUTO_LOGGER, __VA_ARGS__)
#define BASIS_LOG_WARN(...) SPDLOG_LOGGER_WARN(AUTO_LOGGER, __VA_ARGS__)
#define BASIS_LOG_ERROR(...) SPDLOG_LOGGER_ERROR(AUTO_LOGGER, __VA_ARGS__)
#define BASIS_LOG_CRITICAL(...) SPDLOG_LOGGER_CRITICAL(AUTO_LOGGER, __VA_ARGS__)
#define BASIS_LOG_FATAL(...) SPDLOG_LOGGER_CRITICAL(AUTO_LOGGER, __VA_ARGS__)

#define BASIS_LOG_TRACE_NS(ns, ...) SPDLOG_LOGGER_TRACE(ns::AUTO_LOGGER, __VA_ARGS__)
#define BASIS_LOG_DEBUG_NS(ns, ...) SPDLOG_LOGGER_DEBUG(ns::AUTO_LOGGER, __VA_ARGS__)
#define BASIS_LOG_INFO_NS(ns, ...) SPDLOG_LOGGER_INFO(ns::AUTO_LOGGER, __VA_ARGS__)
#define BASIS_LOG_WARN_NS(ns, ...) SPDLOG_LOGGER_WARN(ns::AUTO_LOGGER, __VA_ARGS__)
#define BASIS_LOG_ERROR_NS(ns, ...) SPDLOG_LOGGER_ERROR(ns::AUTO_LOGGER, __VA_ARGS__)
#define BASIS_LOG_CRITICAL_NS(ns, ...) SPDLOG_LOGGER_CRITICAL(ns::AUTO_LOGGER, __VA_ARGS__)
#define BASIS_LOG_FATAL_NS(ns, ...) SPDLOG_LOGGER_CRITICAL(ns::AUTO_LOGGER, __VA_ARGS__)
// #define ASSERT(...)

namespace basis::core::logging {
consteval std::string_view StripLeadingNamespace(std::string_view ns) {
  size_t idx = ns.find_last_of(":");
  if (idx == std::string::npos) {
    idx = 0;
  }
  else {
    idx++;
  }

  ns.remove_prefix(idx);
  return ns;
}
} // namespace basis::core::logging

// TODO: these macros never call spdlog::drop - not a huge deal though

#define DEFINE_AUTO_LOGGER_COMMON                                                                                      \
  extern std::shared_ptr<spdlog::logger> global_logger;                                                                \
  inline const std::shared_ptr<spdlog::logger> &AUTO_LOGGER(global_logger);

#define DEFINE_AUTO_LOGGER_NS(NAMESPACE)                                                                               \
  namespace NAMESPACE {                                                                                                \
  DEFINE_AUTO_LOGGER_COMMON                                                                                            \
  }

#define DECLARE_AUTO_LOGGER_NS(NAMESPACE)                                                                              \
  namespace NAMESPACE {                                                                                                \
  std::shared_ptr<spdlog::logger> global_logger =                                                                      \
      basis::core::logging::CreateLogger(std::string(basis::core::logging::StripLeadingNamespace(#NAMESPACE)));        \
  }

#define DEFINE_AUTO_LOGGER_PLUGIN(PLUGIN_TYPE, PLUGIN_NAME)                                                            \
  namespace basis::plugins::PLUGIN_TYPE ::PLUGIN_NAME {                                                                \
  DEFINE_AUTO_LOGGER_COMMON                                                                                            \
  }

#define DECLARE_AUTO_LOGGER_PLUGIN(PLUGIN_TYPE, PLUGIN_NAME)                                                           \
  namespace basis::plugins::PLUGIN_TYPE ::PLUGIN_NAME {                                                                \
  std::shared_ptr<spdlog::logger> global_logger = basis::core::logging::CreateLogger(#PLUGIN_TYPE "::" #PLUGIN_NAME);  \
  }
