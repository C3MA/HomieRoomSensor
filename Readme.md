## Filesystem
### Configuration
Use the config-example.json from the host folder and create here a config.json file.
### HowTo upload
Start Platform.io
Open a new Atom-Terminal and generate the filesystem with the following command :
```pio run -t buildfs```
Upload this new generated filesystem with:
```pio run -t uploadfs```

### Command pio
Can be found at ```~/.platformio/penv/bin/pio```

# Hardware
ESP8266 version ESP12 was used.

The prototype was based on the Witty board
```
REST         |       TXD
ADC    LDR   |       RXD
CH_PD        |       GPIO05
GPIO16       | BTN   GPIO04
GPIO14       |       GPIO00
GPIO12 RGB-G |       GPIO02
GPIO13 RGB-B | RGB-R GPIO15
VCC          |       GND
            USB
```

The following pins are used:
* GPIO4 PM1006 particle sensor
* GPIO2 WS2812 stripe out of three LEDs, replacing the orignal LEDs at front

# Sources
* [https://github.com/amkuipers/witty Witty pinout]