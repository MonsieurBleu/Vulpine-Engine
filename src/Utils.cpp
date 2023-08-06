#include <iostream>
#include <fstream>
#include <Utils.hpp>

std::string readFile(const std::string& filePath) // Mights just use a C approach instead
{
    std::string content;
    std::ifstream fileStream(filePath, std::ios::in);

    if(!fileStream.is_open()) {
        std::cerr << TERMINAL_ERROR << "Could not read file " << filePath << ". File does not exist." << TERMINAL_RESET<< std::endl;
        return "";
    }

    std::string line = "";
    while(!fileStream.eof()) {
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
    while(i != fileName.rend())
    {
        if(*i == '.') break;

        result = *i + result;

        i++;
    }

    return result;
};

void checkHeap()
{
    int  heapstatus;

    // Check heap status
    heapstatus = _heapchk();
    switch( heapstatus )
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
        <<"ERROR - bad start of heap\n";
        break;

    case _HEAPBADNODE:
        std::cout 
        << TERMINAL_ERROR
        << "ERROR - bad node in heap\n";
        break;
    }

    std::cout << TERMINAL_RESET;
}


clockmicro::time_point benchrono;

void startbenchrono()
{
    benchrono = clockmicro::now();
}

void endbenchrono()
{
    duration elapsed = clockmicro::now() - benchrono;

    std::cout << " in " << TERMINAL_TIMER << elapsed.count() << " ms \n" << TERMINAL_RESET;
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