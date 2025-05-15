#pragma once

#include <spdlog/spdlog.h>
#include <string>
#include <memory>
#include <unordered_map>

class Logger {
public:
    static Logger& instance();

    ~Logger();

    std::shared_ptr<spdlog::logger> getLogger() const {
        return mLogger;
    }

    // 设置日志级别
    void setLogLevel(const std::string& level);

    // 获取当前日志级别
    std::string getLogLevel() const;

    // 配置是否输出到控制台
    void setConsoleOutput(bool enable);

    // 配置是否输出到文件
    void setFileOutput(bool enable, const std::string& logDir = "");

    // 设置日志文件名前缀
    void setLogNamePrefix(const std::string& prefix);

    // 设置单个日志文件最大大小(MB)
    void setMaxFileSize(size_t sizeInMB);

    // 设置最大日志文件数量
    void setMaxFiles(size_t maxFiles);

    // 设置日志格式模式
    void setPattern(const std::string& pattern);

    // 刷新日志
    void flush();

private:
    Logger();

    // 禁用拷贝和移动
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    // 获取当前路径
    std::string getCurrentPath() const;

    // 初始化日志记录器
    void initLogger();

    // 将字符串日志级别转换为spdlog级别
    spdlog::level::level_enum stringToLevel(const std::string& level) const;

    void createDirectoryIfNotExists(const std::string& dirPath);

private:
    std::shared_ptr<spdlog::logger> mLogger;
    std::string mLogDir;
    std::string mLogNamePrefix = "EC20SMS_";
    bool mConsoleOutput = true;
    bool mFileOutput = false;
    std::string mLevel = "debug";
    size_t mMaxFileSize = 10; // 单位：MB
    size_t mMaxFiles = 100;
    std::string mPattern = "[%-6t] [%Y-%m-%d %H:%M:%S.%e] [%!] [%^%-7l%$] %v [%s:%#]";

    // 日志级别映射表
    const std::unordered_map<std::string, spdlog::level::level_enum> mLevelMap = {
        {"trace", spdlog::level::trace},
        {"debug", spdlog::level::debug},
        {"info", spdlog::level::info},
        {"warn", spdlog::level::warn},
        {"error", spdlog::level::err},
        {"critical", spdlog::level::critical},
        {"off", spdlog::level::off}
    };
};

// 带代码行号的日志宏
#define LOG_TRACE(...)      SPDLOG_LOGGER_CALL(Logger::instance().getLogger().get(), spdlog::level::trace, __VA_ARGS__)
#define LOG_DEBUG(...)      SPDLOG_LOGGER_CALL(Logger::instance().getLogger().get(), spdlog::level::debug, __VA_ARGS__)
#define LOG_INFO(...)       SPDLOG_LOGGER_CALL(Logger::instance().getLogger().get(), spdlog::level::info, __VA_ARGS__)
#define LOG_WARN(...)       SPDLOG_LOGGER_CALL(Logger::instance().getLogger().get(), spdlog::level::warn, __VA_ARGS__)
#define LOG_ERROR(...)      SPDLOG_LOGGER_CALL(Logger::instance().getLogger().get(), spdlog::level::err, __VA_ARGS__)
#define LOG_CRITICAL(...)   SPDLOG_LOGGER_CALL(Logger::instance().getLogger().get(), spdlog::level::critical, __VA_ARGS__)

// 不带代码行号的简单日志宏
#define LOG_TRACE_SIMPLE(...)   Logger::instance().getLogger()->trace(__VA_ARGS__)
#define LOG_DEBUG_SIMPLE(...)   Logger::instance().getLogger()->debug(__VA_ARGS__)
#define LOG_INFO_SIMPLE(...)    Logger::instance().getLogger()->info(__VA_ARGS__)
#define LOG_WARN_SIMPLE(...)    Logger::instance().getLogger()->warn(__VA_ARGS__)
#define LOG_ERROR_SIMPLE(...)   Logger::instance().getLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL_SIMPLE(...) Logger::instance().getLogger()->critical(__VA_ARGS__)