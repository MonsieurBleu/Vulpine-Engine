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

#ifdef __clang__
inline thread_local bool currentThreadID = -1;
#elif defined(__GNUC__)
static inline thread_local bool currentThreadID = -1;
#endif


// glm print overloads
std::ostream& operator<<(std::ostream& os, const glm::vec2& u);
std::ostream& operator<<(std::ostream& os, const glm::vec3& u);
std::ostream& operator<<(std::ostream& os, const glm::vec4& u);

std::ostream& operator<<(std::ostream& os, const glm::mat2& u);
std::ostream& operator<<(std::ostream& os, const glm::mat3& u);
std::ostream& operator<<(std::ostream& os, const glm::mat4& u);

std::ostream& operator<<(std::ostream& os, const glm::quat& u);

/// LOGGING

#include "Logging.hpp"
#define FILE_ERROR_MESSAGE(filename, ...) { \
    Logger::setInfo({__PRETTY_FUNCTION__, __FILE__, __LINE__}); \
    Logger::fileError(filename, __VA_ARGS__); \
}

#define WARNING_MESSAGE(...) { \
    Logger::setInfo({__PRETTY_FUNCTION__, __FILE__, __LINE__}); \
    Logger::warn(__VA_ARGS__); \
}

#define ERROR_MESSAGE(...) { \
    Logger::setInfo({__PRETTY_FUNCTION__, __FILE__, __LINE__}); \
    Logger::error(__VA_ARGS__); \
}

#define NOTIF_MESSAGE(...) { \
    Logger::setInfo({__PRETTY_FUNCTION__, __FILE__, __LINE__}); \
    Logger::info(__VA_ARGS__); \
}

// also adding a macro called info cause I never remember it's called notif in this engine :(
#define INFO_MESSAGE(...) { \
    Logger::setInfo({__PRETTY_FUNCTION__, __FILE__, __LINE__}); \
    Logger::info(__VA_ARGS__); \
}

#define DEBUG_MESSAGE(...) { \
    Logger::setInfo({__PRETTY_FUNCTION__, __FILE__, __LINE__}); \
    Logger::debug(__VA_ARGS__); \
}

/// TERMINAL
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

namespace EngineLitterals
{

struct StrHex
{
    uint64_t num;

    constexpr operator uint8_t()
    {
        return (uint8_t)num;
    }

    constexpr operator uint16_t()
    {
        return (uint16_t)num;
    }

    constexpr operator uint32_t()
    {
        return (uint32_t)num;
    }

    constexpr operator uint64_t()
    {
        return (uint64_t)num;
    }
};

namespace litterals
{
    constexpr StrHex inline operator""_hex(const char *s, std::size_t size)
    {
        switch (size)
        {
            case 0:
                return {};
            case 1:
                return {(uint64_t)s[0]};
            case 2:
                return {(uint64_t)(s[0]) | (uint64_t)(s[1]) << 8};
            case 3:
                return {(uint64_t)(s[0]) | (uint64_t)(s[1]) << 8 | (uint64_t)(s[2]) << 16};
            default:
                return {(uint64_t)(s[0]) | (uint64_t)(s[1]) << 8 | (uint64_t)(s[2]) << 16 | (uint64_t)(s[3]) << 24};
        }
    }
};

} // namespace EngineLitterals

using namespace EngineLitterals::litterals;

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

void ensureNonZeroVectorComponents(glm::vec3& v);

// get a specific line from a string, lines are 0-indexed and separated by '\n'. 
// assumes a null-terminated string.
// returns empty string if line not found
std::string getLineFromString(const char* str, size_t line);

#endif