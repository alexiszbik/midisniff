#include "CommandConsole.h"
#include "Logger.h"
#include "MidiFilter.h"
#include "MidiMessageFormatter.h"

#include <RtMidi.h>

#include <iostream>
#include <limits>
#include <string>

namespace
{
    struct MidiCallbackContext
    {
        MidiFilter* filter = nullptr;
    };

    void midiCallback(double deltaTime, std::vector<unsigned char>* message, void* userData)
    {
        const auto* context = static_cast<MidiCallbackContext*>(userData);
        if (context == nullptr || context->filter == nullptr || !context->filter->shouldLog(*message))
            return;

        const std::string formatted = MidiMessageFormatter::format(deltaTime, *message);
        if (!formatted.empty())
            Logger::midi(formatted);
    }

    int readPortSelection(unsigned int portCount)
    {
        while (true)
        {
            Logger::write("Select MIDI input [0-" + std::to_string(portCount - 1) + "]: ");

            int selection = -1;
            if (!(std::cin >> selection))
            {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                Logger::info("Invalid input. Enter a number.");
                continue;
            }

            if (selection < 0 || static_cast<unsigned int>(selection) >= portCount)
            {
                Logger::info("Out of range. Try again.");
                continue;
            }

            return selection;
        }
    }
}

int main()
{
    try
    {
        RtMidiIn midiIn(RtMidi::Api::UNSPECIFIED, "midisniff");
        midiIn.ignoreTypes(false, false, false);

        const unsigned int portCount = midiIn.getPortCount();

        Logger::info("MIDI Sniffer");
        Logger::info("============");

        if (portCount == 0)
        {
            Logger::info("No MIDI input ports found.");
            return 1;
        }

        Logger::info("");
        Logger::info("Available MIDI inputs:");
        for (unsigned int i = 0; i < portCount; ++i)
            Logger::info("  [" + std::to_string(i) + "] " + midiIn.getPortName(i));

        Logger::info("");
        const int selectedPort = readPortSelection(portCount);

        MidiFilter filter;
        MidiCallbackContext context { &filter };

        midiIn.setCallback(&midiCallback, &context);
        midiIn.openPort(static_cast<unsigned int>(selectedPort));

        Logger::info("");
        Logger::info("Logging MIDI from: " + midiIn.getPortName(static_cast<unsigned int>(selectedPort)));
        Logger::info("Type commands below. Use 'help' for options, 'quit' to stop.");
        Logger::info("");

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        CommandConsole console(filter);
        console.printHelp();
        console.start();
        console.join();
    }
    catch (const RtMidiError& error)
    {
        Logger::error("RtMidi error: " + error.getMessage());
        return 1;
    }
    catch (const std::exception& error)
    {
        Logger::error(std::string("Error: ") + error.what());
        return 1;
    }

    return 0;
}
