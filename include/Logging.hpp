#pragma once

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <source_location>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

inline constexpr std::string_view TERMINAL_UNDERLINE = "\033[4m";
inline constexpr std::string_view TERMINAL_ERROR     = "\033[1;31m";
inline constexpr std::string_view TERMINAL_BOLD      = "\033[1m";
inline constexpr std::string_view TERMINAL_INFO      = "\033[94m";
inline constexpr std::string_view TERMINAL_OK        = "\033[92m";
inline constexpr std::string_view TERMINAL_RESET     = "\033[0m";
inline constexpr std::string_view TERMINAL_TIMER     = "\033[93m";
inline constexpr std::string_view TERMINAL_FILENAME  = "\033[95m";
inline constexpr std::string_view TERMINAL_WARNING   = "\033[38;5;208m";
inline constexpr std::string_view TERMINAL_NOTIF     = "\033[36m";
inline constexpr std::string_view TERMINAL_DEBUG     = "\033[35m";

class Logger {
public:
    enum LogSeverity {
        INFO       = 1 << 0,
        WARN       = 1 << 1,
        _ERROR_    = 1 << 2,
        FILE_ERROR = 1 << 3,
        DEBUG      = 1 << 4
    };

    struct Location {
        std::string_view file;
        std::string_view function;
        std::uint_least32_t line = 0;
        std::uint_least32_t column = 0;

        constexpr Location() = default;

        constexpr Location(std::string_view file, std::string_view function,
                           std::uint_least32_t line = 0, std::uint_least32_t column = 0) noexcept
            : file(file), function(function), line(line), column(column) {}

        constexpr Location(const std::source_location& loc) noexcept
            : file(loc.file_name()), function(loc.function_name()),
              line(loc.line()), column(loc.column()) {}
    };

    class Message {
    public:
        template <typename T>
        Message(const T& value, std::source_location loc = std::source_location::current()) noexcept
            : value_(std::addressof(value)), print_(&printAs<T>), loc_(loc) {}

        const std::source_location& location() const noexcept { return loc_; }

        friend std::ostream& operator<<(std::ostream& os, const Message& message)
        {
            message.print_(os, message.value_);
            return os;
        }

    private:
        template <typename T>
        static void printAs(std::ostream& os, const void* value) { os << *static_cast<const T*>(value); }

        const void* value_;
        void (*print_)(std::ostream&, const void*);
        std::source_location loc_;
    };

    class FileName {
    public:
        template <typename T>
            requires std::convertible_to<const T&, std::string_view>
        FileName(const T& name, std::source_location loc = std::source_location::current()) noexcept
            : name_(name), loc_(loc) {}

        std::string_view name() const noexcept { return name_; }
        const std::source_location& location() const noexcept { return loc_; }

    private:
        std::string_view name_;
        std::source_location loc_;
    };

    static inline int LogTermDisplayBits = INFO | WARN | _ERROR_ | FILE_ERROR | DEBUG;

private:
    static inline std::ofstream logFile;
    static inline std::ofstream logFile2;

    static inline std::ostringstream ss;

    static inline std::string logDir = "logs/";

    struct SeverityStyle {
        std::string_view fileTag;
        std::string_view termColor;
        std::string_view termTag;
    };

    static constexpr SeverityStyle styleOf(LogSeverity type) noexcept
    {
        switch (type)
        {
            case INFO:       return {"[INFO]",    TERMINAL_NOTIF,   "NOTIF   |"};
            case WARN:       return {"[WARNING]", TERMINAL_WARNING, "WARNING |"};
            case DEBUG:      return {"[DEBUG]",   TERMINAL_DEBUG,   "DEBUG   |"};
            case _ERROR_:
            case FILE_ERROR: return {"[ERROR]",   TERMINAL_ERROR,   "ERROR   |"};
        }
        return {"[UNKNOWN]", TERMINAL_RESET, "?       |"};
    }

    static void resetBuffer()
    {
        std::string buffer = std::move(ss).str();
        buffer.clear();
        ss.str(std::move(buffer));
        ss.clear();
    }

    static constexpr std::size_t skipEscapeSequence(std::string_view text, std::size_t escape) noexcept
    {
        const auto in = [](char c, char lo, char hi) { return c >= lo && c <= hi; };

        std::size_t i = escape + 1;
        if (i >= text.size())
            return i;

        if (text[i] == '[') // CSI: ESC [ <params 0x30-0x3F> <intermediates 0x20-0x2F> <final 0x40-0x7E>
        {
            ++i;
            while (i < text.size() && in(text[i], 0x30, 0x3F)) ++i;
            while (i < text.size() && in(text[i], 0x20, 0x2F)) ++i;
            if (i < text.size() && in(text[i], 0x40, 0x7E)) ++i;
            return i;
        }

        if (text[i] == ']') // OSC (terminal hyperlinks, etc) ends with BEL or ESC '\'
        {
            for (++i; i < text.size() && text[i] != '\n'; ++i)
            {
                if (text[i] == '\a')
                    return i + 1;
                if (text[i] == '\033' && i + 1 < text.size() && text[i + 1] == '\\')
                    return i + 2;
            }
            return i;
        }

        // Other sequences: optional intermediates, then one final byte 0x30-0x7E
        while (i < text.size() && in(text[i], 0x20, 0x2F)) ++i;
        if (i < text.size() && in(text[i], 0x30, 0x7E)) ++i;
        return i;
    }

    static void writeWithoutStyle(std::ostream& out, std::string_view text)
    {
        std::size_t pos = 0;
        while (true)
        {
            const std::size_t escape = text.find('\033', pos);
            if (escape == std::string_view::npos)
            {
                out << text.substr(pos);
                return;
            }
            out << text.substr(pos, escape - pos);
            pos = skipEscapeSequence(text, escape);
        }
    }

    static void writeToFile(std::ofstream& file, std::string_view stamp, std::string_view line)
    {
        if (!file.good())
            return;

        file << stamp;
        writeWithoutStyle(file, line);
        file << std::flush;
    }

    template <typename... MessageTypes>
    static void logAsType(LogSeverity type, const Location& loc, std::string_view filename,
                          const MessageTypes&... messages)
    {
        const bool should_cout = (type & LogTermDisplayBits) != 0;
        const SeverityStyle style = styleOf(type);

        resetBuffer();

        ss << style.fileTag;
        if (type == FILE_ERROR)
            ss << " while loading file " << filename << "\n        |";
        if (!loc.function.empty())
            ss << " from " << loc.function;
        if (!loc.file.empty())
        {
            ss << " at file " << loc.file;
            if (loc.line > 0)
                ss << ':' << loc.line;
        }
        ss << " : ";

        const std::size_t messagesBegin = ss.view().size();
        ((ss << messages), ...);
        const std::size_t messagesEnd = ss.view().size();
        ss << '\n';

        const std::string_view fullLine = ss.view();
        const std::string_view messageText = fullLine.substr(messagesBegin, messagesEnd - messagesBegin);

        if (should_cout)
        {
            std::cout << style.termColor << style.termTag;
            if (type == FILE_ERROR)
                std::cout << TERMINAL_RESET    << " while loading file "
                          << TERMINAL_FILENAME << filename << '\n'
                          << TERMINAL_ERROR    << "        |";
            if (!loc.function.empty())
                std::cout << TERMINAL_RESET << " from "
                          << TERMINAL_UNDERLINE << TERMINAL_INFO << loc.function
                          << TERMINAL_RESET;
            if (!loc.file.empty())
            {
                std::cout << TERMINAL_RESET << " at file " << TERMINAL_FILENAME << loc.file;
                if (loc.line > 0)
                    std::cout << ':' << loc.line;
            }
            std::cout << style.termColor << "\n        |> " << messageText
                      << TERMINAL_RESET << "\n\n";
        }

        if (logFile.good() || logFile2.good())
        {
            char timeString[std::size("[yyyy-mm-ddThh:mm:ssZ] ")];
            const std::time_t now = std::time(nullptr);
            const std::size_t timeLength = std::strftime(timeString, sizeof timeString,
                                                         "[%FT%TZ] ", std::gmtime(&now));
            const std::string_view stamp(timeString, timeLength);

            writeToFile(logFile, stamp, fullLine);
            writeToFile(logFile2, stamp, fullLine);
        }
    }

public:
    static void init()
    {
        std::error_code ec;
        std::filesystem::create_directories(logDir, ec);

        char fileName[std::size("yyyymmddhhmmss.log")];
        const std::time_t now = std::time(nullptr);
        std::strftime(fileName, sizeof fileName, "%Y%m%d%H%M%S.log", std::gmtime(&now));

        logFile.open(logDir + fileName, std::ios::out | std::ios::trunc);
        logFile2.open(logDir + "latest.log", std::ios::out | std::ios::trunc);

        if (!logFile.good())
            Logger::error("Could not open log file :(");
        else
            Logger::info("Logger initialized :)");

        cleanOldFiles();
    }

    static void cleanOldFiles(std::size_t n_files_to_keep = 5)
    {
        namespace fs = std::filesystem;

        std::error_code ec;
        std::vector<fs::directory_entry> entries;
        for (const fs::directory_entry& entry : fs::recursive_directory_iterator(logDir, ec))
        {
            if (entry.path().extension() == ".log")
                entries.push_back(entry);
        }

        if (entries.size() <= n_files_to_keep)
            return;

        std::sort(entries.begin(), entries.end(),
            [](const fs::directory_entry& a, const fs::directory_entry& b)
            {
                return a.last_write_time() > b.last_write_time();
            });

        for (std::size_t i = n_files_to_keep; i < entries.size(); ++i)
            fs::remove(entries[i].path(), ec);
    }

    template <typename... MessageTypes>
    static void logAt(LogSeverity type, const Location& loc, const MessageTypes&... messages)
    {
        logAsType(type, loc, {}, messages...);
    }

    template <typename... Rest>
    static void log(LogSeverity type, Message first, const Rest&... rest)
    {
        logAsType(type, first.location(), {}, first, rest...);
    }
    static void log(LogSeverity type, std::source_location loc = std::source_location::current())
    {
        logAsType(type, loc, {});
    }

    template <typename... Rest>
    static void info(Message first, const Rest&... rest)
    {
        logAsType(INFO, first.location(), {}, first, rest...);
    }
    static void info(std::source_location loc = std::source_location::current())
    {
        logAsType(INFO, loc, {});
    }

    template <typename... Rest>
    static void warn(Message first, const Rest&... rest)
    {
        logAsType(WARN, first.location(), {}, first, rest...);
    }
    static void warn(std::source_location loc = std::source_location::current())
    {
        logAsType(WARN, loc, {});
    }

    template <typename... Rest>
    static void error(Message first, const Rest&... rest)
    {
        logAsType(_ERROR_, first.location(), {}, first, rest...);
    }
    static void error(std::source_location loc = std::source_location::current())
    {
        logAsType(_ERROR_, loc, {});
    }

    template <typename... Rest>
    static void debug(Message first, const Rest&... rest)
    {
        logAsType(DEBUG, first.location(), {}, first, rest...);
    }
    static void debug(std::source_location loc = std::source_location::current())
    {
        logAsType(DEBUG, loc, {});
    }

    template <typename... MessageTypes>
    static void fileError(FileName file, const MessageTypes&... messages)
    {
        logAsType(FILE_ERROR, file.location(), file.name(), messages...);
    }
};

void handleSegfaultStackTrace(int signal);
