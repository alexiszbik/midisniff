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
    : soloChannel_(0)
{
    for (std::size_t i = 0; i < toIndex(MidiMessageType::Count); ++i)
        enabled_[i] = false;

    enabled_[toIndex(MidiMessageType::NoteOff)]       = true;
    enabled_[toIndex(MidiMessageType::NoteOn)]        = true;
    enabled_[toIndex(MidiMessageType::ProgramChange)] = true;
    enabled_[toIndex(MidiMessageType::ControlChange)] = true;

    for (std::size_t i = 0; i < midi::kChannelCount; ++i)
        channels_[i] = true;
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

int MidiFilter::extractChannelIndex(const std::vector<unsigned char>& message)
{
    if (message.empty() || message[0] >= midi::kSystemStatusMin)
        return -1;

    return static_cast<int>(message[0] & midi::kChannelMask);
}

bool MidiFilter::shouldLog(const std::vector<unsigned char>& message) const
{
    const std::lock_guard<std::mutex> lock(mutex_);

    const MidiMessageType type = classify(message);
    if (type == MidiMessageType::Unknown || type == MidiMessageType::Count)
        return false;

    if (!enabled_[toIndex(type)])
        return false;

    const int channelIndex = extractChannelIndex(message);
    if (channelIndex < 0)
        return true;

    return isChannelAllowed(channelIndex);
}

void MidiFilter::setEnabled(MidiMessageType type, bool enabled)
{
    if (type == MidiMessageType::Unknown || type == MidiMessageType::Count)
        return;

    const std::lock_guard<std::mutex> lock(mutex_);
    enabled_[toIndex(type)] = enabled;
}

bool MidiFilter::isEnabled(MidiMessageType type) const
{
    if (type == MidiMessageType::Unknown || type == MidiMessageType::Count)
        return false;

    const std::lock_guard<std::mutex> lock(mutex_);
    return enabled_[toIndex(type)];
}

bool MidiFilter::isValidChannel(int channel)
{
    return channel >= 1 && channel <= midi::kChannelCount;
}

std::size_t MidiFilter::channelIndex(int channel)
{
    return static_cast<std::size_t>(channel - 1);
}

void MidiFilter::setChannelEnabled(int channel, bool enabled)
{
    if (!isValidChannel(channel))
        return;

    const std::lock_guard<std::mutex> lock(mutex_);
    channels_[channelIndex(channel)] = enabled;
}

bool MidiFilter::isChannelEnabled(int channel) const
{
    if (!isValidChannel(channel))
        return false;

    const std::lock_guard<std::mutex> lock(mutex_);
    return channels_[channelIndex(channel)];
}

void MidiFilter::enableAllChannels()
{
    const std::lock_guard<std::mutex> lock(mutex_);
    for (std::size_t i = 0; i < midi::kChannelCount; ++i)
        channels_[i] = true;
}

void MidiFilter::disableAllChannels()
{
    const std::lock_guard<std::mutex> lock(mutex_);
    for (std::size_t i = 0; i < midi::kChannelCount; ++i)
        channels_[i] = false;
}

void MidiFilter::setSoloChannel(int channel)
{
    if (!isValidChannel(channel))
        return;

    const std::lock_guard<std::mutex> lock(mutex_);
    soloChannel_ = channel;
}

void MidiFilter::clearSolo()
{
    const std::lock_guard<std::mutex> lock(mutex_);
    soloChannel_ = 0;
}

bool MidiFilter::hasSolo() const
{
    const std::lock_guard<std::mutex> lock(mutex_);
    return soloChannel_ > 0;
}

int MidiFilter::soloChannel() const
{
    const std::lock_guard<std::mutex> lock(mutex_);
    return soloChannel_;
}

bool MidiFilter::isChannelAllowed(int channelIndex) const
{
    if (soloChannel_ > 0)
        return channelIndex == soloChannel_ - 1;

    return channels_[static_cast<std::size_t>(channelIndex)];
}
