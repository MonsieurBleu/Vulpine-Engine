#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <iomanip>

#include <string>
#include <chrono>
#include <glm/glm.hpp>


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

#define ERROR_MESSAGE(message) std::cerr \
    << TERMINAL_ERROR << "ERROR \xBF from " \
    << TERMINAL_UNDERLINE << __PRETTY_FUNCTION__ \
    << TERMINAL_RESET << TERMINAL_ERROR << " at file " << __FILE__ << ":" << __LINE__ \
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

#endif