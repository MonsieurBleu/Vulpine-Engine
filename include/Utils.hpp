#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <iomanip>

#include <string>
#include <chrono>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>


#ifdef _WIN32
    #include <shlwapi.h>
    #define STR_CASE_STR(str1, str2) StrStrIA(str1, str2)
#else
    #define STR_CASE_STR(str1, str2) strcasestr(str1, str2)
#endif


/// TERMINAL
const std::string TERMINAL_UNDERLINE = "\033[4m";
const std::string TERMINAL_ERROR     = "\e[1;31m";
const std::string TERMINAL_BOLD      = "\e[1m";
const std::string TERMINAL_INFO      = "\033[94m";
const std::string TERMINAL_OK        = "\033[92m";
const std::string TERMINAL_RESET     = "\033[0m";
const std::string TERMINAL_TIMER     = "\033[93m";
const std::string TERMINAL_FILENAME  = "\033[95m";
const std::string TERMINAL_WARNING   = "\e[38;5;208m";
const std::string TERMINAL_NOTIF     = "\e[36m";


#define FILE_ERROR_MESSAGE(filename, message) std::cerr     \
    << TERMINAL_ERROR     << "ERROR   |"                    \
    << TERMINAL_RESET     << " while loading file "         \
    << TERMINAL_FILENAME  << filename << "\n"               \
    << TERMINAL_ERROR     << "        |"                    \
    << TERMINAL_RESET     << " from "                       \
    << TERMINAL_UNDERLINE                                   \
    << TERMINAL_INFO      << __PRETTY_FUNCTION__            \
    << TERMINAL_RESET                                       \
    << TERMINAL_RESET     << " at file "                    \
    << TERMINAL_FILENAME  << __FILE__ << ":" << __LINE__    \
    << TERMINAL_ERROR     << "\n        |> " << message     \
    << TERMINAL_RESET     << "\n\n";

#define WARNING_MESSAGE(message) std::cerr                  \
    << TERMINAL_WARNING   << "WARNING |"                    \
    << TERMINAL_RESET     << " from "                       \
    << TERMINAL_UNDERLINE                                   \
    << TERMINAL_INFO      << __PRETTY_FUNCTION__            \
    << TERMINAL_RESET                                       \
    << TERMINAL_RESET     << " at file "                    \
    << TERMINAL_FILENAME  << __FILE__ << ":" << __LINE__    \
    << TERMINAL_WARNING   << "\n        |> " << message     \
    << TERMINAL_RESET     << "\n\n";


#define ERROR_MESSAGE(message) std::cerr                    \
    << TERMINAL_ERROR     << "ERROR   |"                    \
    << TERMINAL_RESET     << " from "                       \
    << TERMINAL_UNDERLINE                                   \
    << TERMINAL_INFO      << __PRETTY_FUNCTION__            \
    << TERMINAL_RESET                                       \
    << TERMINAL_RESET     << " at file "                    \
    << TERMINAL_FILENAME  << __FILE__ << ":" << __LINE__    \
    << TERMINAL_ERROR     << "\n        |> " << message     \
    << TERMINAL_RESET     << "\n\n";

#define NOTIF_MESSAGE(message) std::cerr                    \
    << TERMINAL_NOTIF     << "NOTIF   |"                    \
    << TERMINAL_RESET     << " from "                       \
    << TERMINAL_UNDERLINE                                   \
    << TERMINAL_INFO      << __PRETTY_FUNCTION__            \
    << TERMINAL_RESET                                       \
    << TERMINAL_RESET     << " at file "                    \
    << TERMINAL_FILENAME  << __FILE__ << ":" << __LINE__    \
    << TERMINAL_NOTIF     << "\n        |> " << message     \
    << TERMINAL_RESET     << "\n\n";

#ifdef _WIN32
    // #include <shlwapi.h>
    // #define strcasestr StrStrI
    #include <string.h>
#endif

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

void replace(std::string &str, const std::string &substr, const std::string &newsubstr);

inline constexpr double fromDayMonth(int day, int month)
{
    return (day + (month - 1) * 30) / 365.0 + 0.25;
}

bool fileExists(const char *path);

// rgba user defined literal
// source: https://stackoverflow.com/questions/66813961/c-constexpr-constructor-for-colours
namespace colours
{
struct Colour
{
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;
    std::uint8_t a;

    constexpr operator glm::vec3() const
    {
        return {r / 255.0f, g / 255.0f, b / 255.0f};
    }
    constexpr operator glm::vec4() const
    {
        return {r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f};
    }
};

// helper to display values
std::ostream inline &operator<<(std::ostream &os, const Colour &c)
{
    std::ostringstream oss;
    oss << std::hex << std::setfill('0') << '{' << std::setw(2) << static_cast<int>(c.r) << ',' << std::setw(2)
        << static_cast<int>(c.g) << ',' << std::setw(2) << static_cast<int>(c.b) << ',' << std::setw(2)
        << static_cast<int>(c.a) << '}';
    return os << oss.str();
}

// decode a nibble
constexpr inline std::uint8_t nibble(char n)
{
    if (n >= '0' && n <= '9')
        return n - '0';
    return n - 'a' + 10;
}

// decode a byte
constexpr inline std::uint8_t byte(const char *b)
{
    return nibble(b[0]) << 4 | nibble(b[1]);
}

// User-defined literals - These don't care if you start with '#' or
// if the strings have the correct length.

constexpr int roff = 1; // offsets in C strings
constexpr int goff = 3;
constexpr int boff = 5;
constexpr int aoff = 7;

namespace literals
{
constexpr Colour inline operator""_rgb(const char *s, std::size_t)
{
    return {byte(s + roff), byte(s + goff), byte(s + boff), 0xff};
}

constexpr Colour inline operator""_rgba(const char *s, std::size_t)
{
    return {byte(s + roff), byte(s + goff), byte(s + boff), byte(s + aoff)};
}
}
} // namespace colours

using namespace colours::literals;


// glm print overloads
std::ostream& operator<<(std::ostream& os, const glm::vec2& u);
std::ostream& operator<<(std::ostream& os, const glm::vec3& u);
std::ostream& operator<<(std::ostream& os, const glm::vec4& u);

std::ostream& operator<<(std::ostream& os, const glm::mat2& u);
std::ostream& operator<<(std::ostream& os, const glm::mat3& u);
std::ostream& operator<<(std::ostream& os, const glm::mat4& u);

std::ostream& operator<<(std::ostream& os, const glm::quat& u);

inline void str2lower(char* s) {
    for (; *s; ++s) *s = tolower(*s);
}

inline void str2upper(char* s) {
    for (; *s; ++s) *s = toupper(*s);
}

inline bool isInteger(const std::string& s, int& value) {
    if (s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false;

    char* p;
    value = strtol(s.c_str(), &p, 10);

    return (*p == 0);
}

inline bool isFloat(const std::string& s, float& value) {
    if (s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+') && (s[0] != '.'))) return false;

    char* p;
    value = strtof(s.c_str(), &p);

    return (*p == 0);
}

void angleVectors(const glm::vec3& angles, glm::vec3& forward, glm::vec3& right, glm::vec3& up);


// get a specific line from a string, lines are 0-indexed and separated by '\n'. 
// assumes a null-terminated string.
// returns empty string if line not found
std::string getLineFromString(const char* str, size_t line);

#endif