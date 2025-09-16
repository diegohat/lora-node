# LoRa Node (Sensor Transmitter)

Reference README for a LoRa sensor/transmitter node designed to work with the companion gateway repo `lora-gateway`. It sends compact LoRa frames that the gateway parses and relays to MQTT.

This document is intended to live in the separate Node repository: https://github.com/diegohat/lora-node

Gateway repository for reference: https://github.com/diegohat/lora-gateway

## Hardware
- MCU + LoRa radio used in this repo:
  - Arduino Uno + SX1276/SX1278 module
- Antenna matching your regional frequency
- Sensors (examples used in code):
  - Soil hygrometer (digital DO + analog AO)
  - Tilt sensor (digital)

### Pinout and Wiring (Arduino Uno)
- LoRa pins (as used in `src/main.cpp`):
  - `SS` 10, `RST` 9, `DIO0` 2
- Sensors:
  - Hygrometer DO → `D3`, AO → `A0`
  - Tilt sensor → `D4`

## Message Format
The gateway expects a one-byte address followed by a UTF-8 text payload containing fields in Portuguese separated by commas:

- First byte: destination address (use the gateway `myAddress` or `0xFF` for broadcast)
- Text payload example produced by this node:
  - `Solo=SECO,Umidade=73.00%,Inclinacao=NORMAL`

Fields parsed by the gateway and forwarded to MQTT JSON:
- `Solo`: soil condition
- `Umidade`: soil moisture (may include `%` symbol)
- `Inclinacao`: tilt state

Tip: Keep payloads short to reduce airtime and collisions.

## LoRa Settings (must match gateway)
- Frequency: default `915E6` (change per region: 433/868/915 MHz)
- Spreading Factor: `SF12`
- Coding Rate: `4/8`

To interoperate, the node must use identical frequency/SF/CR (and ideally bandwidth) as the gateway.

## Example (minimal Arduino snippet)
```cpp
byte dest = 0x01; // or 0xFF for broadcast
String payload = "Solo=123,Umidade=45%,Inclinacao=2";

LoRa.beginPacket();
LoRa.write(dest);         // address byte first
LoRa.print(payload);      // text payload
LoRa.endPacket();
```

## Power & Duty Cycle
- Respect regional regulations (e.g., duty-cycle, maximum EIRP)
- Use sleep modes to extend battery life (this repo uses `rocketscream/Low-Power` to sleep ~15 minutes between transmissions)

## PlatformIO
This repo includes a PlatformIO configuration for Arduino Uno:

```ini
[env:uno]
platform = atmelavr
board = uno
framework = arduino
monitor_speed = 9600

lib_deps =
  sandeepmistry/LoRa
  rocketscream/Low-Power
```

### Build, Upload, Monitor (macOS/zsh)
```sh
pio run
pio run -t upload
pio device monitor -b 9600
```

Ensure wiring matches the pins above and your antenna is connected before powering.

## Testing
- With the gateway running, send a packet and watch the gateway serial monitor and MQTT topic (default `diegohat/lora`).
- Verify that JSON arrives with `solo`, `umidade`, `inclinacao`, `rssi`, and `snr`.

## Adapting Fields
If your sensor names differ, either:
- Update the node to send `Solo`, `Umidade`, and `Inclinacao`, or
- Modify the gateway parser in `lora-gateway/src/main.cpp` to match your field names.

## Repository Pairing
- Gateway repo: firmware receives LoRa and publishes to MQTT over TLS
- Node repo: this transmitter (sends address byte + text payload)

Keep both READMEs aligned so team members can configure frequency, addressing, and formats consistently.
