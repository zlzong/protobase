#pragma once

#include <spdlog/spdlog.h>

class Logger {
public:
    static Logger& instance();

    ~Logger();

    std::shared_ptr<spdlog::logger> getLogger() {
        return mLogger;
    }

    void setLogLevel(std::string level);

private:
    Logger();
    std::string get_current_path();
private:
    std::shared_ptr<spdlog::logger> mLogger;
    std::string mLogDir = get_current_path();
    std::string mLogNamePrefix = "nss_";
    bool mConsole = true;
    std::string mLevel = "debug";
};

// 有代码行号
 #define LOG_TRACE(...)      SPDLOG_LOGGER_CALL(Logger::instance().getLogger().get(), spdlog::level::trace, __VA_ARGS__)
 #define LOG_DEBUG(...)      SPDLOG_LOGGER_CALL(Logger::instance().getLogger().get(), spdlog::level::debug, __VA_ARGS__)
 #define LOG_INFO(...)       SPDLOG_LOGGER_CALL(Logger::instance().getLogger().get(), spdlog::level::info, __VA_ARGS__)
 #define LOG_WARN(...)       SPDLOG_LOGGER_CALL(Logger::instance().getLogger().get(), spdlog::level::warn, __VA_ARGS__)
 #define LOG_ERROR(...)      SPDLOG_LOGGER_CALL(Logger::instance().getLogger().get(), spdlog::level::err, __VA_ARGS__)
 #define LOG_CRITICAL(...)   SPDLOG_LOGGER_CALL(Logger::instance().getLogger().get(), spdlog::level::critical, __VA_ARGS__)

// 没有代码行号
//#define LOG_TRACE(...)       Logger::instance().getLogger().get()->trace(__VA_ARGS__)
//#define LOG_DEBUG(...)       Logger::instance().getLogger().get()->debug(__VA_ARGS__)
//#define LOG_INFO(...)        Logger::instance().getLogger().get()->info(__VA_ARGS__)
//#define LOG_WARN(...)        Logger::instance().getLogger().get()->warn(__VA_ARGS__)
//#define LOG_ERROR(...)       Logger::instance().getLogger().get()->error(__VA_ARGS__)
//#define LOG_CRITICAL(...)    Logger::instance().getLogger().get()->critical(__VA_ARGS__)
