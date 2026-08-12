#pragma once

#include <iosfwd>
#include <mutex>
#include <string>

class Logger
{
public:
    static void info(const std::string& message);
    static void error(const std::string& message);
    static void midi(const std::string& message);
    static void write(const std::string& message);

private:
    static std::mutex& mutex();
    static void writeln(std::ostream& stream, const std::string& message);
    static void writeTo(std::ostream& stream, const std::string& message);
};
