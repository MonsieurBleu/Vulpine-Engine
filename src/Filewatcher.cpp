#include <Filewatcher.hpp>
#include <Utils.hpp>

Filewatcher::Filewatcher()
{}

Filewatcher::Filewatcher(const std::string & path)
    : path(path)
{

    if(std::filesystem::exists(path)) {
        lastWriteTime = std::filesystem::last_write_time(path);
    }
    else
    {
        ERROR_MESSAGE("Trying to create Filewatcher for a non existant file '" << path << "'")
    }
}

bool Filewatcher::hasChanged()
{
    if(path.empty()) return false;

    if(!std::filesystem::exists(path))
    {
        WARNING_MESSAGE("Filewatcher created for file '" << path << "' can't track modifications. The file no longer exist.");
        return false;
    }

    auto newWriteTime = std::filesystem::last_write_time(path);
    if (newWriteTime != lastWriteTime) {
        lastWriteTime = newWriteTime;
        return true;
    }

    return false;
}


const std::string Filewatcher::getPath() const{return path;};