#include <iostream>
#include <fstream>
#include <algorithm>
#include <Utils.hpp>

std::string readFile(const std::string &filePath) // Mights just use a C approach instead
{
    std::string content;
    std::ifstream fileStream(filePath, std::ios::in);
 
    if (!fileStream.is_open())
    {
        std::cerr << TERMINAL_ERROR << "Could not read file " << filePath << ". File does not exist." << TERMINAL_RESET << std::endl;
        return "";
    }

    std::string line = "";
    while (!fileStream.eof())
    {
        std::getline(fileStream, line);
        content.append(line + "\n");
    }

    fileStream.close();
    return content;
}

std::string getFileExtension(const std::string &fileName)
{
    std::string result;

    auto i = fileName.rbegin();
    while (i != fileName.rend())
    {
        if (*i == '.')
            break;

        result = *i + result;

        i++;
    }

    return result;
}

const char* getFileExtensionC(const char* fileName)
{   
    int i = 0;
    while(fileName[i] != '\0') i++;

    while(i > 0 && fileName[i] != '.')
        i--;
    
    return fileName + i + (i ? 1 : 0);
}

void checkHeap()
{
#ifdef _WIN32
    int heapstatus;

    // Check heap status
    heapstatus = _heapchk();
    switch (heapstatus)
    {
    case _HEAPOK:
        std::cout
            << TERMINAL_OK
            << " OK - heap is fine\n";
        break;
    case _HEAPEMPTY:

        std::cout
            << TERMINAL_OK
            << " OK - heap is empty\n";
        break;

    case _HEAPBADBEGIN:
        std::cout
            << TERMINAL_ERROR
            << "ERROR - bad start of heap\n";
        break;

    case _HEAPBADNODE:
        std::cout
            << TERMINAL_ERROR
            << "ERROR - bad node in heap\n";
        break;
    }

    std::cout << TERMINAL_RESET;
#endif
}

clockmicro::time_point benchrono;

void startbenchrono()
{
    benchrono = clockmicro::now();
}

void endbenchrono()
{
    duration elapsed = clockmicro::now() - benchrono;

    std::cout << " in " << TERMINAL_TIMER << elapsed.count() << " ms \n"
              << TERMINAL_RESET;
}

uint64_t GetTimeMs()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

clockmicro::time_point last_deltatime;

double Get_delta_time()
{
    clockmicro::time_point now = clockmicro::now();
    duration delta = now - last_deltatime;
    last_deltatime = now;
    return delta.count() * 0.001;
}

std::string getFileNameFromPath(const char *path)
{
    std::string name;
    int i = 0;

    for(; path[i] != '\0'; i++);

    for(; i >= 0 && path[i] != '/' && path[i] != '\\'; i--)
        name.push_back(path[i]);
    
    std::reverse(name.begin(), name.end());

    return name;
}

std::string getNameOnlyFromPath(const char *path)
{
    std::string name = getFileNameFromPath(path);
    std::string res;

    int i = 0;
    for(; name[i] != '\0' && name[i] != '.' ; i++)
        res += name[i];

    return res;
}

bool fileExists(const char *path)
{
    std::ifstream file(path);
    return file.good(); 
}