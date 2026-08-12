#pragma once

#include <cstddef>
#include <string>
#include <vector>

class MidiMessageFormatter
{
public:
    static std::string format(double deltaTime, const std::vector<unsigned char>& message);

private:
    static std::string formatRawBytes(const std::vector<unsigned char>& message);
    static std::string decodeMessage(const std::vector<unsigned char>& message);
    static std::string noteName(unsigned char note);
};
