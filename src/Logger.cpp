#include "Logger.h"

#include <iostream>

std::mutex& Logger::mutex()
{
    static std::mutex instance;
    return instance;
}

void Logger::writeln(std::ostream& stream, const std::string& message)
{
    stream << message << std::endl;
}

void Logger::writeTo(std::ostream& stream, const std::string& message)
{
    stream << message;
    stream.flush();
}

void Logger::info(const std::string& message)
{
    const std::lock_guard<std::mutex> lock(mutex());
    writeln(std::cout, message);
}

void Logger::error(const std::string& message)
{
    const std::lock_guard<std::mutex> lock(mutex());
    writeln(std::cerr, message);
}

void Logger::midi(const std::string& message)
{
    const std::lock_guard<std::mutex> lock(mutex());
    writeln(std::cout, message);
}

void Logger::write(const std::string& message)
{
    const std::lock_guard<std::mutex> lock(mutex());
    writeTo(std::cout, message);
}
