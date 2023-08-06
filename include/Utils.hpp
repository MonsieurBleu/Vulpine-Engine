#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>

#include <string>
#include <chrono>


/// TERMINAL
const std::string TERMINAL_ERROR    = "\e[1;31m"; //"\033[91m";
const std::string TERMINAL_INFO     = "\033[94m";
const std::string TERMINAL_OK       = "\033[92m";
const std::string TERMINAL_RESET    = "\033[0m";
const std::string TERMINAL_TIMER    = "\033[93m";
const std::string TERMINAL_FILENAME = "\033[95m";
const std::string TERMINAL_WARNING  = "\e[38;5;208m";
const std::string TERMINAL_NOTIF    = "\e[1;36m";


/// FILES
/*
    TODO : replace with a faster version, using C functions (like in readOBJ)
*/
std::string readFile(const std::string& filePath);
std::string getFileExtension(const std::string &fileName);


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

#endif