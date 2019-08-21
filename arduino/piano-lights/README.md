# set up arduino-mk

```
sudo apt-get install build-essential
sudo apt-get install arduino arduino-core arduino-mk
```

--via http://www.raspberryvi.org/stories/arduino-cli.html

# install needed libraries

Ensure [Adafruit_NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel) is in `/usr/share/arduino/libraries/`

# upload to the arduino from the pi

```
make upload
```

# notes

* This works great with an (authentic) Arduino Uno at baud rate 560800. Tried with an Arduino Nano V3 clone (`BOARD_TAG = nano328`, `ARDUINO_PORT = /dev/ttyUSB0`), but ran into some issues. First, any baud rate other than 9600 (tried 39400, 115200, 560800) would cause the strip to appear as if it's missing almost all note signals from the pi, despite the serial comms looking normal. Second, it caused noticeable lag in both Fluidsynth output and the lights on the strip. I suspect both problems are related to the USB-to-serial chip on the Nano clone (CH340G, vs. FT232RL on authentic Nanos). Want to try with a real Nano next.
