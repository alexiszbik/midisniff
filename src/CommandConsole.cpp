#include "CommandConsole.h"

#include "Logger.h"
#include "MidiConstants.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace
{
    std::string normalizeToken(std::string token)
    {
        token.erase(std::remove(token.begin(), token.end(), '_'), token.end());
        token.erase(std::remove(token.begin(), token.end(), '-'), token.end());
        std::transform(token.begin(), token.end(), token.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return token;
    }

    std::vector<std::string> splitWords(const std::string& line)
    {
        std::istringstream stream(line);
        std::vector<std::string> words;
        std::string word;
        while (stream >> word)
            words.push_back(word);
        return words;
    }

    bool parseMessageType(const std::string& token, MidiMessageType& type)
    {
        const std::string normalized = normalizeToken(token);

        if (normalized == "noteon")
        {
            type = MidiMessageType::NoteOn;
            return true;
        }
        if (normalized == "noteoff")
        {
            type = MidiMessageType::NoteOff;
            return true;
        }
        if (normalized == "cc" || normalized == "controlchange")
        {
            type = MidiMessageType::ControlChange;
            return true;
        }
        if (normalized == "pc" || normalized == "programchange")
        {
            type = MidiMessageType::ProgramChange;
            return true;
        }

        return false;
    }

    const char* messageTypeLabel(MidiMessageType type)
    {
        switch (type)
        {
            case MidiMessageType::NoteOn:        return "note on";
            case MidiMessageType::NoteOff:       return "note off";
            case MidiMessageType::ControlChange: return "control change";
            case MidiMessageType::ProgramChange: return "program change";
            default:                             return "message";
        }
    }

    bool parseChannel(const std::string& token, int& channel)
    {
        try
        {
            channel = std::stoi(token);
            return channel >= 1 && channel <= midi::kChannelCount;
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    std::string onOffLabel(bool enabled)
    {
        return enabled ? "on" : "off";
    }
}

CommandConsole::CommandConsole(MidiFilter& filter)
    : filter_(filter)
{
}

void CommandConsole::printHelp() const
{
    Logger::info("Commands:");
    Logger::info("  help                         Show this help");
    Logger::info("  status                       Show current filter state");
    Logger::info("  quit                         Stop the application");
    Logger::info("  ch on <1-16|all>             Enable one or all channels");
    Logger::info("  ch off <1-16|all>            Disable one or all channels");
    Logger::info("  solo <1-16>                  Solo one channel");
    Logger::info("  unsolo                       Clear solo mode");
    Logger::info("  type on <noteon|noteoff|cc|pc>   Enable a message category");
    Logger::info("  type off <noteon|noteoff|cc|pc>  Disable a message category");
}

void CommandConsole::start()
{
    if (running_.exchange(true))
        return;

    thread_ = std::thread(&CommandConsole::run, this);
}

void CommandConsole::stop()
{
    running_ = false;
}

void CommandConsole::join()
{
    if (thread_.joinable())
        thread_.join();
}

void CommandConsole::run()
{
    std::string line;
    while (running_ && std::getline(std::cin, line))
    {
        if (!running_)
            break;

        if (!handleCommand(line))
            break;
    }
}

bool CommandConsole::handleCommand(const std::string& line)
{
    const std::vector<std::string> words = splitWords(line);
    if (words.empty())
        return true;

    const std::string command = normalizeToken(words[0]);

    if (command == "help" || command == "h" || command == "?")
    {
        printHelp();
        return true;
    }

    if (command == "quit" || command == "exit" || command == "q")
        return false;

    if (command == "status")
    {
        printStatus();
        return true;
    }

    if (command == "unsolo")
    {
        filter_.clearSolo();
        Logger::info("Solo cleared.");
        return true;
    }

    if (command == "solo")
    {
        if (words.size() < 2)
        {
            Logger::info("Usage: solo <1-16>");
            return true;
        }

        int channel = 0;
        if (!parseChannel(words[1], channel))
        {
            Logger::info("Channel must be between 1 and " + std::to_string(midi::kChannelCount) + ".");
            return true;
        }

        filter_.setSoloChannel(channel);
        Logger::info("Solo channel " + std::to_string(channel) + ".");
        return true;
    }

    if (command == "ch")
    {
        if (words.size() < 3)
        {
            Logger::info("Usage: ch on|off <1-16|all>");
            return true;
        }

        const std::string action = normalizeToken(words[1]);
        const std::string target = normalizeToken(words[2]);

        if (action != "on" && action != "off")
        {
            Logger::info("Usage: ch on|off <1-16|all>");
            return true;
        }

        const bool enable = action == "on";

        if (target == "all")
        {
            if (enable)
                filter_.enableAllChannels();
            else
                filter_.disableAllChannels();

            Logger::info(std::string("All channels ") + (enable ? "enabled." : "disabled."));
            return true;
        }

        int channel = 0;
        if (!parseChannel(target, channel))
        {
            Logger::info("Channel must be between 1 and " + std::to_string(midi::kChannelCount) + " or 'all'.");
            return true;
        }

        filter_.setChannelEnabled(channel, enable);
        Logger::info("Channel " + std::to_string(channel) + " " + (enable ? "enabled." : "disabled."));
        return true;
    }

    if (command == "type")
    {
        if (words.size() < 3)
        {
            Logger::info("Usage: type on|off <noteon|noteoff|cc|pc>");
            return true;
        }

        const std::string action = normalizeToken(words[1]);
        if (action != "on" && action != "off")
        {
            Logger::info("Usage: type on|off <noteon|noteoff|cc|pc>");
            return true;
        }

        MidiMessageType type = MidiMessageType::Unknown;
        if (!parseMessageType(words[2], type))
        {
            Logger::info("Unknown message type. Use noteon, noteoff, cc or pc.");
            return true;
        }

        filter_.setEnabled(type, action == "on");
        Logger::info(std::string(messageTypeLabel(type)) + " logging " + onOffLabel(action == "on") + ".");
        return true;
    }

    Logger::info("Unknown command. Type 'help' for available commands.");
    return true;
}

void CommandConsole::printStatus() const
{
    Logger::info("Filter status:");

    Logger::info("  Message types:");
    Logger::info("    note on: " + onOffLabel(filter_.isEnabled(MidiMessageType::NoteOn)));
    Logger::info("    note off: " + onOffLabel(filter_.isEnabled(MidiMessageType::NoteOff)));
    Logger::info("    control change: " + onOffLabel(filter_.isEnabled(MidiMessageType::ControlChange)));
    Logger::info("    program change: " + onOffLabel(filter_.isEnabled(MidiMessageType::ProgramChange)));

    if (filter_.hasSolo())
    {
        Logger::info("  Channels: solo " + std::to_string(filter_.soloChannel()));
        return;
    }

    std::string enabledChannels;
    bool allEnabled = true;
    for (int channel = 1; channel <= midi::kChannelCount; ++channel)
    {
        if (filter_.isChannelEnabled(channel))
        {
            if (!enabledChannels.empty())
                enabledChannels += ", ";

            enabledChannels += std::to_string(channel);
        }
        else
        {
            allEnabled = false;
        }
    }

    if (enabledChannels.empty())
        Logger::info("  Channels: none enabled");
    else if (allEnabled)
        Logger::info("  Channels: all enabled");
    else
        Logger::info("  Channels enabled: " + enabledChannels);
}
