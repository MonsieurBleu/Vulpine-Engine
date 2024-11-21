#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>

#include <string>
#include <chrono>


/// TERMINAL
const std::string TERMINAL_UNDERLINE = "\033[4m";
const std::string TERMINAL_ERROR    = "\e[1;31m";
const std::string TERMINAL_INFO     = "\033[94m";
const std::string TERMINAL_OK       = "\033[92m";
const std::string TERMINAL_RESET    = "\033[0m";
const std::string TERMINAL_TIMER    = "\033[93m";
const std::string TERMINAL_FILENAME = "\033[95m";
const std::string TERMINAL_WARNING  = "\e[38;5;208m";
const std::string TERMINAL_NOTIF    = "\e[1;36m";


#define FILE_ERROR_MESSAGE(filename, message) std::cerr \
    << TERMINAL_ERROR << "Error \xBF loading file " \
    << TERMINAL_TIMER << filename \
    << TERMINAL_ERROR << " from "\
    << TERMINAL_UNDERLINE << __PRETTY_FUNCTION__ \
    << TERMINAL_RESET << TERMINAL_ERROR << " at file " << __FILE__ << ":" << __LINE__ \
    << "\n      \xC0\xC4\xC4 "\
    << TERMINAL_WARNING << message \
    << TERMINAL_RESET << "\n\n";

#define WARNING_MESSAGE(message) std::cerr \
    << TERMINAL_WARNING << "WARNING \xBF from " \
    << TERMINAL_UNDERLINE << __PRETTY_FUNCTION__ \
    << TERMINAL_RESET << TERMINAL_WARNING << " at file " << __FILE__ << ":" << __LINE__ \
    << "\n        \xC0\xC4\xC4 "\
    << TERMINAL_WARNING << message \
    << TERMINAL_RESET << "\n\n";

/// FILES
/*
    TODO : replace with a faster version, using C functions (like in readOBJ)
*/
std::string readFile(const std::string& filePath);
std::string getFileExtension(const std::string &fileName);
const char* getFileExtensionC(const char* fileName);


/// CHRONO
typedef std::chrono::high_resolution_clock clockmicro;
typedef std::chrono::duration<float, std::milli> duration;

void startbenchrono();
void endbenchrono();
uint64_t GetTimeMs();

double Get_delta_time();

/// HEAP CORRUPTION
void checkHeap();

/// VECTORS

// #include <glm/glm.hpp>
// #include <glm/gtx/string_cast.hpp>

// std::ostream& operator<<(std::ostream& os, const glm::vec2& u)
// {
//     os << "vec2(" << u.x << ", " << u.y << ")";
//     return os;
// };

// #include <glm/glm.hpp>

// std::ostream& operator<<(std::ostream& os, const glm::vec3& u)
// {
//     os << "vec3(" << u.x << ", " << u.y << ", " << u.z << ")";
//     return os;
// };

// std::ostream& operator<<(std::ostream& os, const glm::vec4& u)
// {
//     os << "vec2(" << u.x << ", " << u.y << ", " << u.z << ", " << u.w << ")";
//     return os;
// };


// template<typename genType>
// std::ostream& operator<<(std::ostream& out, const genType& g)
// {
//     return out << glm::to_string(g);
// }

std::string getFileNameFromPath(const char *path);
std::string getNameOnlyFromPath(const char *path);

inline constexpr double fromDayMonth(int day, int month)
{
    return (day + (month - 1) * 30) / 365.0 + 0.25;
}

#endif