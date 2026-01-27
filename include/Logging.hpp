#pragma once

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>


const std::string TERMINAL_UNDERLINE = "\033[4m";
const std::string TERMINAL_ERROR     = "\e[1;31m";
const std::string TERMINAL_BOLD      = "\e[1m";
const std::string TERMINAL_INFO      = "\033[94m";
const std::string TERMINAL_OK        = "\033[92m";
const std::string TERMINAL_RESET     = "\033[0m";
const std::string TERMINAL_TIMER     = "\033[93m";
const std::string TERMINAL_FILENAME  = "\033[95m";
const std::string TERMINAL_WARNING   = "\e[38;5;208m";
const std::string TERMINAL_NOTIF     = "\e[36m";
const std::string TERMINAL_DEBUG     = "\e[35m";

class Logger {
public:
    enum LogSeverity {
        INFO       = 1 << 0,
        WARN       = 1 << 1,
        ERROR      = 1 << 2,
        FILE_ERROR = 1 << 3,
        DEBUG      = 1 << 4
    };

    struct LogInfo {
        const char* function;
        const char* file;
        int line;
        const char* filename;
    };

    static inline int LogTermDisplayBits = INFO | WARN | ERROR | FILE_ERROR | DEBUG;

private:

    static inline std::fstream logFile;

    static inline std::stringstream ss;


    static void log_part(bool should_cout) {}; // default case for recursive unwrapping of arguments
    
    template <typename T, typename... Types>
    static void log_part(bool should_cout, T message, Types... remainder)
    {
        ss << message;

        if (should_cout)
            std::cout << message;

        log_part(should_cout, remainder...);
    }

    
    static inline LogInfo logInfo {nullptr, nullptr, 0, nullptr};

    template <typename... MessageTypes>
    static void logAsType(LogSeverity type, const MessageTypes&... messages)
    {
        bool should_cout = (type & LogTermDisplayBits) != 0;
        ss = std::stringstream();
        switch (type)
        {
            case LogSeverity::INFO:
                ss  << "[INFO]";
                if (should_cout)
                    std::cout  << TERMINAL_NOTIF     << "NOTIF   |";
                break;
            case LogSeverity::WARN:
                ss  << "[WARNING]";
                if (should_cout)
                    std::cout  << TERMINAL_WARNING   << "WARNING |";
                break;
            case LogSeverity::ERROR:
                ss  << "[ERROR]";
                if (should_cout)
                    std::cout  << TERMINAL_ERROR     << "ERROR   |";
                break;
            case LogSeverity::DEBUG:
                ss  << "[DEBUG]";
                if (should_cout)
                    std::cout  << TERMINAL_DEBUG     << "DEBUG   |";
                break;
                break;
            case LogSeverity::FILE_ERROR:
                ss  << "[ERROR]"            \
                    << " while loading file " \
                    << logInfo.filename << "\n"  \
                    << "        |";

                if (should_cout)
                    std::cout   << TERMINAL_ERROR     << "ERROR   |"     \
                                << TERMINAL_RESET     << " while loading file " \
                                << TERMINAL_FILENAME  << logInfo.filename << "\n"  \
                                << TERMINAL_ERROR     << "        |";
                break;
        }

        if (logInfo.function != nullptr)
        {
            ss << " from " << logInfo.function;

            if (should_cout)    
                std::cout   << TERMINAL_RESET     << " from "      \
                            << TERMINAL_UNDERLINE                  \
                            << TERMINAL_INFO      << logInfo.function \
                            << TERMINAL_RESET;
        }

        if (logInfo.file != nullptr)
        {
            ss << " at file " << logInfo.file;

            if (should_cout)
                std::cout  << TERMINAL_RESET     << " at file "                    \
                           << TERMINAL_FILENAME  << logInfo.file;

            if (logInfo.line > 0)
            {
                ss  << ":" << logInfo.line;
                if (should_cout)
                    std::cout  << ":" << logInfo.line;
            }
        }
        
        if (should_cout)
        {
            switch (type)
            {
            case LogSeverity::INFO:
                std::cout  << TERMINAL_NOTIF;
                break;
            case LogSeverity::WARN:
                std::cout  << TERMINAL_WARNING;
                break;
            case LogSeverity::DEBUG:
                std::cout  << TERMINAL_DEBUG;
                break;
            case LogSeverity::ERROR:
            case LogSeverity::FILE_ERROR:
                std::cout  << TERMINAL_ERROR;
                break;
            }
        }

        ss  << " : ";
        if (should_cout)
            std::cout  << "\n        |> ";

        log_part(should_cout, messages...);


        ss << "\n";
        if (should_cout)
            std::cout  << TERMINAL_RESET     << "\n\n";


        const std::string str = ss.str();
        if (logFile.good())
        {
            std::time_t time = std::time({});
            char timeString[std::size("[yyyy-mm-ddThh:mm:ssZ] ")];
            std::strftime(std::data(timeString), std::size(timeString), "[%FT%TZ] ", std::gmtime(&time));

            logFile << timeString;
            
            logFile << str << std::flush;
        }
    }

public:
    static void init()
    {
        std::time_t time = std::time({});
        char timeString[std::size("data/logs/yyyymmddhhmmss.vulpine.log")];
        std::strftime(std::data(timeString), std::size(timeString), "data/logs/%Y%m%d%H%M%S.vulpine.log", std::gmtime(&time));
        logFile.open(timeString, std::ios::out | std::ios::trunc);

        if (!logFile.good())
        {
            Logger::error("Could not open log file :(");
        }
        else 
        {
            Logger::debug("Logger initialized :)");
        }
    }

    // Loggers

    template <typename... MessageTypes>
    static void log(LogSeverity type, const MessageTypes&... messages)
    {
        switch (type)
        {
            case INFO:
                logAsType(INFO, messages...);
                break;
            case FILE_ERROR:
                logAsType(FILE_ERROR, messages...);
                break;
            case ERROR:
                logAsType(ERROR, messages...);
                break;
            case WARN:
                logAsType(WARN, messages...);
                break;
            case DEBUG:
                logAsType(DEBUG, messages...);
                break;
        }
    }

    template <typename... MessageTypes>
    static void info(const MessageTypes&... messages)
    {
        logAsType(LogSeverity::INFO, messages...);
    }

    template <typename... MessageTypes>
    static void warn(const MessageTypes&... messages)
    {
        logAsType(LogSeverity::WARN, messages...);
    }

    template <typename... MessageTypes>
    static void error(const MessageTypes&... messages)
    {
        logAsType(LogSeverity::ERROR, messages...);
    }

    template <typename... MessageTypes>
    static void debug(const MessageTypes&... messages)
    {
        logAsType(LogSeverity::DEBUG, messages...);
    }


    template <typename... MessageTypes>
    static void fileError(const char* filename, const MessageTypes&... messages)
    {
        logInfo.filename = filename;
        logAsType(LogSeverity::FILE_ERROR, messages...);
    }

    template <typename... MessageTypes>
    static void fileError(std::string filename, const MessageTypes&... messages)
    {
        logInfo.filename = filename.c_str();
        logAsType(LogSeverity::FILE_ERROR, messages...);
    }

    // Setters
    static void setFunction(const char* function)
    {
        logInfo.function = function;
    }

    static void setFile(const char* file)
    {
        logInfo.file = file;
    }
    
    static void setLine(int line)
    {
        logInfo.line = line;
    }
    
    static void setFilename(const char* filename)
    {
        logInfo.filename = filename;
    }
    
    static void setInfo(const LogInfo& newInfo)
    {
        logInfo = newInfo;
    }
};