#pragma once

#include <filesystem>
#include <string>

/* CREDITS : 
    Base template for this module by Luna Bossu https://github.com/ClairDeLuna01
*/

class Filewatcher {
private:
    std::filesystem::file_time_type lastWriteTime;
    std::string path;

public:
    Filewatcher();
    Filewatcher(const std::string & path);

    bool hasChanged();
    const std::string getPath() const;
};

