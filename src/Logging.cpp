#include "Logging.hpp"

#include <csignal>
#include <stacktrace>

void handleSegfaultStackTrace(int signal)
{
    static bool called = false;
    if (!called)
    {
        auto stacktrace = std::stacktrace::current();
        
        std::stacktrace_entry entry;
        
        for (size_t i = 2; i < stacktrace.size(); i++)
        {
            if (stacktrace[i].source_file().empty()) continue;
            
            entry = stacktrace[i];
            break;
        }
        
        Logger::LogInfo info;
        std::string source_file = entry.source_file();
        std::string description = entry.description();
        info.file = source_file.c_str();
        info.function = description.c_str();
        info.line = entry.source_line();

        Logger::setInfo(info);

        std::stringstream ss;
        ss << "Unknown Signal " << signal;
        std::string s = ss.str();
        const char* signalStr = s.c_str();
        switch (signal)
        {
            case SIGSEGV:
                signalStr = "SIGSEGV";
                break;
            case SIGBUS:
                signalStr = "SIGBUS";
                break;
            case SIGKILL:
                signalStr = "SIGKILL";
                break;
            case SIGABRT:
                signalStr = "SIGABRT";
        }

        // maybe a little risky, could cause deadlock or data corruption or something but eh ¯\_(ツ)_/¯
        // also, sorry if you're reading this exactly because it happened to you
        // good luck!
        Logger::error("FATAL ERROR: received Signal ", signalStr, "!\nStacktrace: ", '\n', stacktrace);
    }
    called = true;

    _exit(EXIT_FAILURE);
}