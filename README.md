## Real-Time Audio Logging

This repository contains a simple C++ WAV file audio logger, which provides a logging function suitable for running in a real-time audio context.  In addition to the base WavLogger class, there is a light wrapper ThreadedWavLogger, which owns a lower-priority file-writing thread to read from the WavLogger and output to a WAV file.  These classes should be directly importable into any C++ project using C++17 or later - and earlier C++ versions by replacing the std::filesystem functionality with an OS-specific API.

The repository as contains a brief test file an makefile.  Test by running ```make```.  The test program should create a directory of WAV files, logging a sine wave.