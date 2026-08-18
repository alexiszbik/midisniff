#pragma once

#include "MidiFilter.h"

#include <atomic>
#include <string>
#include <thread>

class CommandConsole
{
public:
    explicit CommandConsole(MidiFilter& filter);

    void printHelp() const;
    void start();
    void stop();
    void join();

private:
    void run();
    bool handleCommand(const std::string& line);
    void printStatus() const;

    MidiFilter& filter_;
    std::thread thread_;
    std::atomic<bool> running_ { false };
};
