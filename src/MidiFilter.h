#pragma once

#include "MidiConstants.h"

#include <cstddef>
#include <mutex>
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
    static int extractChannelIndex(const std::vector<unsigned char>& message);

    bool shouldLog(const std::vector<unsigned char>& message) const;

    void setEnabled(MidiMessageType type, bool enabled);
    bool isEnabled(MidiMessageType type) const;

    void setChannelEnabled(int channel, bool enabled);
    bool isChannelEnabled(int channel) const;
    void enableAllChannels();
    void disableAllChannels();

    void setSoloChannel(int channel);
    void clearSolo();
    bool hasSolo() const;
    int soloChannel() const;

private:
    static bool isValidChannel(int channel);
    static std::size_t channelIndex(int channel);
    bool isChannelAllowed(int channelIndex) const;

    mutable std::mutex mutex_;
    bool enabled_[static_cast<std::size_t>(MidiMessageType::Count)];
    bool channels_[midi::kChannelCount];
    int soloChannel_;
};
