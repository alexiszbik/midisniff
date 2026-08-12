#pragma once

#include <cstdint>

namespace midi
{
    constexpr std::uint8_t kStatusNibbleMask = 0xF0;
    constexpr std::uint8_t kChannelMask      = 0x0F;
    constexpr std::uint8_t kSystemStatusMin  = 0xF0;

    constexpr std::uint8_t kNotesPerOctave   = 12;
    constexpr std::uint8_t kVelocityOff      = 0;
    constexpr std::uint8_t kPitchBendLsbBits = 7;

    namespace Channel
    {
        constexpr std::uint8_t kNoteOff          = 0x80;
        constexpr std::uint8_t kNoteOn           = 0x90;
        constexpr std::uint8_t kPolyPressure     = 0xA0;
        constexpr std::uint8_t kControlChange      = 0xB0;
        constexpr std::uint8_t kProgramChange      = 0xC0;
        constexpr std::uint8_t kChannelPressure    = 0xD0;
        constexpr std::uint8_t kPitchBend          = 0xE0;
    }

    namespace SystemCommon
    {
        constexpr std::uint8_t kSysEx            = 0xF0;
        constexpr std::uint8_t kMidiTimeCode     = 0xF1;
        constexpr std::uint8_t kSongPosition     = 0xF2;
        constexpr std::uint8_t kSongSelect       = 0xF3;
        constexpr std::uint8_t kTuneRequest      = 0xF6;
        constexpr std::uint8_t kSysExEnd         = 0xF7;
    }

    namespace SystemRealtime
    {
        constexpr std::uint8_t kTimingClock      = 0xF8;
        constexpr std::uint8_t kStart             = 0xFA;
        constexpr std::uint8_t kContinue          = 0xFB;
        constexpr std::uint8_t kStop              = 0xFC;
        constexpr std::uint8_t kActiveSensing     = 0xFE;
        constexpr std::uint8_t kSystemReset       = 0xFF;
    }
}
