#include "MidiMessageFormatter.h"

#include "MidiConstants.h"

#include <iomanip>
#include <sstream>

namespace
{
    constexpr const char* kNoteNames[] = {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
    };
}

std::string MidiMessageFormatter::format(double deltaTime, const std::vector<unsigned char>& message)
{
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(3) << deltaTime << "s | "
           << formatRawBytes(message) << " | "
           << decodeMessage(message);
    return stream.str();
}

std::string MidiMessageFormatter::formatRawBytes(const std::vector<unsigned char>& message)
{
    std::ostringstream stream;
    for (std::size_t i = 0; i < message.size(); ++i)
    {
        if (i > 0)
            stream << ' ';

        stream << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
               << static_cast<int>(message[i]) << std::dec;
    }
    return stream.str();
}

std::string MidiMessageFormatter::noteName(unsigned char note)
{
    const int octave = static_cast<int>(note) / midi::kNotesPerOctave - 1;
    return std::string(kNoteNames[note % midi::kNotesPerOctave]) + std::to_string(octave);
}

std::string MidiMessageFormatter::decodeMessage(const std::vector<unsigned char>& message)
{
    if (message.empty())
        return "Empty message";

    const unsigned char status = message[0];

    if (status >= midi::kSystemStatusMin)
    {
        switch (status)
        {
            case midi::SystemCommon::kSysEx:        return "System Exclusive";
            case midi::SystemCommon::kMidiTimeCode: return message.size() > 1 ? "MIDI Time Code: " + std::to_string(message[1]) : "MIDI Time Code";
            case midi::SystemCommon::kSongPosition: return message.size() > 2 ? "Song Position: " + std::to_string((message[1] | (message[2] << midi::kPitchBendLsbBits))) : "Song Position";
            case midi::SystemCommon::kSongSelect:   return message.size() > 1 ? "Song Select: " + std::to_string(message[1]) : "Song Select";
            case midi::SystemCommon::kTuneRequest:  return "Tune Request";
            case midi::SystemRealtime::kTimingClock: return "Timing Clock";
            case midi::SystemRealtime::kStart:      return "Start";
            case midi::SystemRealtime::kContinue:   return "Continue";
            case midi::SystemRealtime::kStop:       return "Stop";
            case midi::SystemRealtime::kActiveSensing: return "Active Sensing";
            case midi::SystemRealtime::kSystemReset:   return "System Reset";
            default:                                return "System message";
        }
    }

    const unsigned char channel = (status & midi::kChannelMask) + 1;
    const unsigned char type = status & midi::kStatusNibbleMask;

    switch (type)
    {
        case midi::Channel::kNoteOff:
            if (message.size() < 3)
                return "Note Off (incomplete)";
            return "Note Off ch" + std::to_string(channel) + " "
                   + noteName(message[1]) + " vel " + std::to_string(message[2]);

        case midi::Channel::kNoteOn:
            if (message.size() < 3)
                return "Note On (incomplete)";
            if (message[2] == midi::kVelocityOff)
                return "Note Off ch" + std::to_string(channel) + " "
                       + noteName(message[1]) + " vel 0";
            return "Note On ch" + std::to_string(channel) + " "
                   + noteName(message[1]) + " vel " + std::to_string(message[2]);

        case midi::Channel::kPolyPressure:
            if (message.size() < 3)
                return "Poly Pressure (incomplete)";
            return "Poly Pressure ch" + std::to_string(channel) + " "
                   + noteName(message[1]) + " pressure " + std::to_string(message[2]);

        case midi::Channel::kControlChange:
            if (message.size() < 3)
                return "Control Change (incomplete)";
            return "CC ch" + std::to_string(channel) + " "
                   + std::to_string(message[1]) + " val " + std::to_string(message[2]);

        case midi::Channel::kProgramChange:
            if (message.size() < 2)
                return "Program Change (incomplete)";
            return "Program Change ch" + std::to_string(channel) + " prog " + std::to_string(message[1]);

        case midi::Channel::kChannelPressure:
            if (message.size() < 2)
                return "Channel Pressure (incomplete)";
            return "Channel Pressure ch" + std::to_string(channel) + " pressure " + std::to_string(message[1]);

        case midi::Channel::kPitchBend:
            if (message.size() < 3)
                return "Pitch Bend (incomplete)";
            {
                const int value = (message[2] << midi::kPitchBendLsbBits) | message[1];
                return "Pitch Bend ch" + std::to_string(channel) + " value " + std::to_string(value);
            }

        default:
            return "Unknown channel message";
    }
}
