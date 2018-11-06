# NFCeyboard

This project implements the firmware for a simple key-board that reads NFC tags
attached to key ring and reports their presence to an MQTT server. Multiple MFRC522
based, SPI connected NFC readers are supported.

# Configuration

Before building the project, copy `src/config.h.template` to `src/config.h` and fill in
the following details.

* The Wifi SSID and passphrase
* The MQTT server to connect to
* The NFC tags to listen to, along with the MQTT topic to use for reporting
* The SS (chip-select) pins for each MFRC522 reader

# Building the software

The project is implemented with PlatformIO. Please make sure to use a PlatformIO enabled
environment when compiling.

# Hardware wiring

The firmware is compatible to ESP8266 board. To interface the NFC readers, connect them
to the SPI pins of the hardware so that they all share the `MISO`, `MOSI` and `CLK` lines,
along with `VCC` (3.3V) and `GND`. Each NFC reader needs to be connected to a dedicated `SS`
(chip-select) line. The configuration in `config.h` needs to specify these chip-select pins.

# HomeAssistant

As the firmware communicates with its environment via an MQTT server, it can be easily
integrated into HomeAssistant installations.

Add your MQTT broker to `configuration.yaml`. For instance:

```
mqtt:
  broker: localhost
```

And then add a device for each key that was configured in the firmware. For instance:

```
notify:
- platform: mqtt
  devices:
    foo_key: 'key/foo'
```

For more information, refer to the [HomeAssistant MQTT documentation](https://www.home-assistant.io/components/device_tracker.mqtt/).