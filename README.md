# Velox

## Build Instructions

### Setup

ESP-IDF is used so will need to be installed

#### Dependencies
- cmake
- esptool
- esp-idf

#### Window
Install instructions can be found [here](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/windows-setup.html)

#### Linux
If nix is used then there is a provided flake.nix that will handle all dependencies

## Usage
Run `make menuconfig` to access to configuration options


`make flash` will compile and flash the firmware to the ESP32

`make monitor` will start the monitor to readback any messages from the ESP32

Under "General Settings" there are the options for the web and LoRa configuration.

### Lora
Under the LoRa section it is possible to modify the settings for
- Sync Word
- Frequency
- Spreading Factor
- Bandwidth
- Coding Rate
- Preamble Length
- Implicit Header
- CRC
- Invert IQ

### Web
Under the Web section it is possible to modify the settings for
- SSID
- WiFi Password
- Channel
- Max connections

## Modifying
To modify the ground station to accept other
data then it is only necessary to modify `config.h` and `web_files.h`

`config.h` contains the raw struct that is used
for decoding the received messages. The contents of this
should match what is being sent.

The contents of render_oled represent what will be displayed on the OLED
screen.

format_json is used to display the contents on the web server that is
hosted

`web_files.h` represents the UI of the web server. This
is a `RAWSTRING` that should represent some HTML code.

The code that is featured was used for Panthera, but can be modified for
other rockets. For those rockets, additional branches should be
made to which the changes are commited
