# Audio Recorder

This is a simple C program that records audio using the ALSA (Advanced Linux Sound Architecture) API and saves it as a WAV file. The project also includes a Makefile for compiling and building the program.

## Features

- Records audio from the default ALSA audio device.
- Saves the recording as a WAV file.
- Configurable recording duration, sample rate, and channels.

## Requirements

Make sure you have the following installed on your system:

- GCC (GNU Compiler Collection)
- ALSA library development package (`libasound2-dev`)

To install the required ALSA development library on a Debian-based system:

```bash
sudo apt-get install libasound2-dev
