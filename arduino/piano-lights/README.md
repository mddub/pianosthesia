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
