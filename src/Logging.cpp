#include "Logging.hpp"

#include <csignal>
#include <cstdlib>

#ifndef _WIN32
    #include <stacktrace>
#endif

namespace
{
    constexpr std::string_view signalName(int signal) noexcept
    {
        switch (signal)
        {
            case SIGSEGV: return "SIGSEGV"; // segmentation fault (invalid memory reference)
            case SIGABRT: return "SIGABRT"; // abort (sent by os, basically a nicer sigkill that asks the program to stop)
            case SIGFPE:  return "SIGFPE";  // floating point error
            case SIGILL:  return "SIGILL";  // illegal instruction
        #ifdef SIGBUS
            case SIGBUS:  return "SIGBUS";  // bus error (bad memory access)
        #endif
            default:      return "unknown signal";
        }
    }
}

void handleSegfaultStackTrace(int signal)
{
    static volatile std::sig_atomic_t called = 0;
    if (called)
        std::_Exit(EXIT_FAILURE);
    called = 1;

#ifdef _WIN32
    /* TODO : do a windows compatible version */
    Logger::error("FATAL ERROR: received signal ", signalName(signal), " (", signal, ")");
#else
    const auto stacktrace = std::stacktrace::current();

    std::string sourceFile;
    std::string function;
    std::uint_least32_t line = 0;

    for (std::size_t i = 2; i < stacktrace.size(); ++i)
    {
        sourceFile = stacktrace[i].source_file();
        if (sourceFile.empty())
            continue;

        function = stacktrace[i].description();
        line = stacktrace[i].source_line();
        break;
    }

    // maybe a little risky, could cause deadlock or data corruption or something but eh ¯\_(ツ)_/¯
    // also, sorry if you're reading this exactly because it happened to you
    // good luck!
    Logger::logAt(Logger::_ERROR_, {sourceFile, function, line},
                  "FATAL ERROR: received signal ", signalName(signal), " (", signal, ")!\n"
                  "Stacktrace:\n", stacktrace);
#endif

    std::cout.flush();
    std::_Exit(EXIT_FAILURE);
}
