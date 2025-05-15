#include "Logger.h"
#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <iostream>
#include <sstream>

Logger &Logger::instance() {
    static Logger zInstance;
    return zInstance;
}

std::string Logger::getCurrentPath() const {
    char temp[PATH_MAX];
    if (getcwd(temp, PATH_MAX) != nullptr) {
        return std::string(temp);
    }
    return std::string(".");
}

Logger::Logger() : mLogDir(getCurrentPath()) {
    try {
        // 初始化异步日志处理
        spdlog::init_thread_pool(8192, 1);

        // 初始化日志记录器
        initLogger();
    } catch (const spdlog::spdlog_ex &ex) {
        std::cerr << "[Logger] Initialization failed: " << ex.what() << std::endl;
    }
    catch (const std::exception &ex) {
        std::cerr << "[Logger] General error during initialization: " << ex.what() << std::endl;
    }
}

Logger::~Logger() {
    try {
        if (mLogger) {
            mLogger->flush();
        }
        spdlog::shutdown();
    } catch (const std::exception &ex) {
        std::cerr << "[Logger] Error during shutdown: " << ex.what() << std::endl;
    }
}

void Logger::createDirectoryIfNotExists(const std::string &dirPath) {
    if (dirPath.empty()) {
        return;
    }

    std::string currentPath;
    std::istringstream pathStream(dirPath);
    std::string token;

    // 处理根目录
    if (dirPath[0] == '/') {
        currentPath = "/";
    }

    while (std::getline(pathStream, token, '/')) {
        if (token.empty()) {
            continue;
        }

        currentPath += token + "/";
        mkdir(currentPath.c_str(), 0755);
    }
}

void Logger::initLogger() {
    try {
        // 移除旧的logger如果存在
        spdlog::drop(mLogNamePrefix + "log");

        // 创建sink列表
        std::vector<spdlog::sink_ptr> sinks;

        // 添加控制台输出sink
        if (mConsoleOutput) {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_level(stringToLevel(mLevel));
            console_sink->set_pattern(mPattern);
            sinks.push_back(console_sink);
        }

        // 添加文件输出sink
        if (mFileOutput) {
            // 确保日志目录存在 (C++11 兼容方式)
            createDirectoryIfNotExists(mLogDir);

            // 构建日志文件路径
            std::string logFilePath = mLogDir;
            // 确保路径末尾有分隔符
            if (!logFilePath.empty() && logFilePath.back() != '/' && logFilePath.back() != '\\') {
                logFilePath += "/";
            }
            logFilePath += mLogNamePrefix + "log.log";

            // 创建文件sink（按大小旋转）
            auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                logFilePath, mMaxFileSize * 1024 * 1024, mMaxFiles);
            file_sink->set_level(stringToLevel(mLevel));
            file_sink->set_pattern(mPattern);
            sinks.push_back(file_sink);
        }

        // 没有可用的sink时，默认添加控制台sink
        if (sinks.empty()) {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_level(stringToLevel(mLevel));
            console_sink->set_pattern(mPattern);
            sinks.push_back(console_sink);
        }

        // 创建多sink异步日志记录器
        mLogger = std::make_shared<spdlog::async_logger>(
            mLogNamePrefix + "log",
            sinks.begin(), sinks.end(),
            spdlog::thread_pool(),
            spdlog::async_overflow_policy::block
        );

        // 设置日志级别
        mLogger->set_level(stringToLevel(mLevel));
        mLogger->flush_on(stringToLevel(mLevel));

        // 注册为全局日志记录器
        spdlog::register_logger(mLogger);
    } catch (const std::exception &ex) {
        std::cerr << "[Logger] Error creating logger: " << ex.what() << std::endl;

        // 创建一个简单的控制台记录器作为后备
        mLogger = spdlog::stdout_color_mt(mLogNamePrefix + "log");
        mLogger->set_pattern(mPattern);
        mLogger->set_level(spdlog::level::debug);
    }
}

void Logger::setLogLevel(const std::string &level) {
    try {
        mLevel = level;
        if (mLogger) {
            auto levelEnum = stringToLevel(level);
            mLogger->set_level(levelEnum);
            mLogger->flush_on(levelEnum);
        }
    } catch (const std::exception &ex) {
        std::cerr << "[Logger] Error setting log level: " << ex.what() << std::endl;
    }
}

std::string Logger::getLogLevel() const {
    return mLevel;
}

void Logger::setConsoleOutput(bool enable) {
    if (mConsoleOutput != enable) {
        mConsoleOutput = enable;
        initLogger(); // 重新创建logger以应用新配置
    }
}

void Logger::setFileOutput(bool enable, const std::string &logDir) {
    bool needReinit = (mFileOutput != enable);

    mFileOutput = enable;

    if (!logDir.empty() && mLogDir != logDir) {
        mLogDir = logDir;
        needReinit = true;
    }

    if (needReinit) {
        initLogger(); // 重新创建logger以应用新配置
    }
}

void Logger::setLogNamePrefix(const std::string &prefix) {
    if (mLogNamePrefix != prefix) {
        mLogNamePrefix = prefix;
        initLogger(); // 重新创建logger以应用新配置
    }
}

void Logger::setMaxFileSize(size_t sizeInMB) {
    if (mMaxFileSize != sizeInMB) {
        mMaxFileSize = sizeInMB;
        if (mFileOutput) {
            initLogger(); // 仅当使用文件输出时才需要重新初始化
        }
    }
}

void Logger::setMaxFiles(size_t maxFiles) {
    if (mMaxFiles != maxFiles) {
        mMaxFiles = maxFiles;
        if (mFileOutput) {
            initLogger(); // 仅当使用文件输出时才需要重新初始化
        }
    }
}

void Logger::setPattern(const std::string &pattern) {
    if (mPattern != pattern) {
        mPattern = pattern;
        if (mLogger) {
            mLogger->set_pattern(pattern);
        }
    }
}

void Logger::flush() {
    if (mLogger) {
        mLogger->flush();
    }
}

spdlog::level::level_enum Logger::stringToLevel(const std::string &level) const {
    auto it = mLevelMap.find(level);
    if (it != mLevelMap.end()) {
        return it->second;
    }
    // 默认返回debug级别
    return spdlog::level::debug;
}
