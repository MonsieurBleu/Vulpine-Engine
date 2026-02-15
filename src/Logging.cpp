#include "Logging.hpp"

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
        Logger::error("ERROR: received Segmentation Fault!\nStacktrace: ", '\n', stacktrace);
    }
    called = true;

    exit(1);
}