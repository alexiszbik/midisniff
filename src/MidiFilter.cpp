#include "MidiFilter.h"

#include "MidiConstants.h"

namespace
{
    std::size_t toIndex(MidiMessageType type)
    {
        return static_cast<std::size_t>(type);
    }
}

MidiFilter::MidiFilter()
{
    for (std::size_t i = 0; i < toIndex(MidiMessageType::Count); ++i) {
        enabled_[i] = false;
    }

    enabled_[toIndex(MidiMessageType::NoteOff)]       = true;
    enabled_[toIndex(MidiMessageType::NoteOn)]        = true;
    enabled_[toIndex(MidiMessageType::ProgramChange)] = true;
    enabled_[toIndex(MidiMessageType::ControlChange)] = true;
}

MidiMessageType MidiFilter::classify(const std::vector<unsigned char>& message)
{
    if (message.empty())
        return MidiMessageType::Unknown;

    const unsigned char status = message[0];

    if (status >= midi::kSystemStatusMin)
    {
        if (status == midi::SystemCommon::kSysEx)
            return MidiMessageType::SystemExclusive;

        if (status >= midi::SystemRealtime::kTimingClock)
            return MidiMessageType::SystemRealtime;

        return MidiMessageType::SystemCommon;
    }

    const unsigned char type = status & midi::kStatusNibbleMask;

    switch (type)
    {
        case midi::Channel::kNoteOff:
            return MidiMessageType::NoteOff;

        case midi::Channel::kNoteOn:
            if (message.size() >= 3 && message[2] == midi::kVelocityOff)
                return MidiMessageType::NoteOff;
            return MidiMessageType::NoteOn;

        case midi::Channel::kPolyPressure:
            return MidiMessageType::PolyPressure;

        case midi::Channel::kControlChange:
            return MidiMessageType::ControlChange;

        case midi::Channel::kProgramChange:
            return MidiMessageType::ProgramChange;

        case midi::Channel::kChannelPressure:
            return MidiMessageType::ChannelPressure;

        case midi::Channel::kPitchBend:
            return MidiMessageType::PitchBend;

        default:
            return MidiMessageType::Unknown;
    }
}

bool MidiFilter::shouldLog(const std::vector<unsigned char>& message) const
{
    const MidiMessageType type = classify(message);
    if (type == MidiMessageType::Unknown || type == MidiMessageType::Count)
        return false;

    return enabled_[toIndex(type)];
}

void MidiFilter::setEnabled(MidiMessageType type, bool enabled)
{
    if (type == MidiMessageType::Unknown || type == MidiMessageType::Count)
        return;

    enabled_[toIndex(type)] = enabled;
}

bool MidiFilter::isEnabled(MidiMessageType type) const
{
    if (type == MidiMessageType::Unknown || type == MidiMessageType::Count)
        return false;

    return enabled_[toIndex(type)];
}
