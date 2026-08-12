#pragma once

#include <cstddef>
#include <vector>

enum class MidiMessageType
{
    NoteOff,
    NoteOn,
    PolyPressure,
    ControlChange,
    ProgramChange,
    ChannelPressure,
    PitchBend,
    SystemExclusive,
    SystemCommon,
    SystemRealtime,
    Unknown,
    Count
};

class MidiFilter
{
public:
    MidiFilter();

    static MidiMessageType classify(const std::vector<unsigned char>& message);

    bool shouldLog(const std::vector<unsigned char>& message) const;

    void setEnabled(MidiMessageType type, bool enabled);
    bool isEnabled(MidiMessageType type) const;

private:
    bool enabled_[static_cast<std::size_t>(MidiMessageType::Count)];
};
