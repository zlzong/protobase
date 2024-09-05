#include "Logger.h"
#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <iostream>

Logger& Logger::instance() {
    static Logger zInstance;
    return zInstance;
}

std::string Logger::get_current_path() {
    char temp[PATH_MAX];
    if (getcwd(temp, PATH_MAX) != nullptr) {
        return std::string(temp);
    }
    return std::string();
}

Logger::Logger() {
    try {
        const std::string loggerName = mLogNamePrefix + std::string("log");
        if (mConsole) {
            mLogger = spdlog::stdout_color_mt(loggerName);
        } else {
            // multi part log files, with every part 50M, max 100 files
            mLogger = spdlog::create_async<spdlog::sinks::rotating_file_sink_mt>(
                    loggerName, mLogDir.append(loggerName + ".log"), 50 * 1024 * 1024, 100);
        }

        setLogLevel(mLevel);

        mLogger->set_pattern("[%-6t] [%Y-%m-%d %H:%M:%S.%e] [%^%-7l%$] %v [%s:%#] [%!]");
    } catch (const spdlog::spdlog_ex &ex) {
        std::cout << "[Logger] Initialization failed: " << ex.what() << std::endl;
    }
}

Logger::~Logger() {
    spdlog::shutdown();
}

void Logger::setLogLevel(std::string level) {
    mLevel = level;

    if (mLevel == "trace") {
        mLogger->set_level(spdlog::level::trace);
        mLogger->flush_on(spdlog::level::trace);
    } else if (mLevel == "debug") {
        mLogger->set_level(spdlog::level::debug);
        mLogger->flush_on(spdlog::level::debug);
    } else if (mLevel == "info") {
        mLogger->set_level(spdlog::level::info);
        mLogger->flush_on(spdlog::level::info);
    } else if (mLevel == "warn") {
        mLogger->set_level(spdlog::level::warn);
        mLogger->flush_on(spdlog::level::warn);
    } else if (mLevel == "error") {
        mLogger->set_level(spdlog::level::err);
        mLogger->flush_on(spdlog::level::err);
    } else if (mLevel == "critical") {
        mLogger->set_level(spdlog::level::critical);
        mLogger->flush_on(spdlog::level::critical);
    }
}
