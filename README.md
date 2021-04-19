# mac2rpi_coreaudio_plugin
Solution to stream audio from a Mac to a Raspberry Pi. A [playback server](https://github.com/betabandido/mac2rpi_playback_server) running on the RPI listens to the data sent by this plugin and plays it on the audio device.

## Requirements

This solution uses [Boost](https://www.boost.org) C++ libraries. Therefore, they must be installed in your system. Use [Homebrew](https://brew.sh) to easily install them.

## Build

Open the project with XCode and adjust the header and libraries path for Boost as needed. Then build the project and create an archive. Once the archive is ready, select "Distribute Content" and choose "Built Products." Export the built products to your desired location. Then copy the contents of the "Products" folder into the root folder of your system. In the end, the audio plugin should be located at `/Library/Audio/Plug-Ins/HAL/mac2rpi-coreaudio-plugin.driver`.
