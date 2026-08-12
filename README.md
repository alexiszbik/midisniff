# midisniff

Terminal MIDI sniffer based on [RtMidi](https://github.com/thestk/rtmidi).

## Build

```bash
cmake -B build
cmake --build build
```

## Run

```bash
./build/midisniff
```

On startup, the app lists all MIDI inputs and asks you to pick one by number. MIDI messages are then logged to the terminal until you press Enter.

This is project was created to sniff all the songs I made with my MPC1000 to transfer them in an other system. This only log Notes, Program Changes and Control Changes. You can change this by editing the MidiFilter constructor.
