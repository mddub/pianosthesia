# setup

To `/etc/rc.local` add the following

```
KEYSTATION=$(aconnect -l | grep client | grep Keystation | awk '{ print $2 }' | grep -o '[0-9]\+')
FLUIDSYNTH=$(aconnect -l | grep client | grep FLUID | awk '{ print $2 }' | grep -o '[0-9]\+')
aconnect $KEYSTATION:0 $FLUIDSYNTH:0
```

To /etc/init.d/fluidsynth add the `fluidsynth.init` file.


## start synth

fluidsynth --audio-driver=alsa --gain 3 /usr/share/sounds/sf2/FluidR3_GM.sf2 -c=8 -z=32

aconnect -l
aconnect 20:0 128:0
ps aux | grep synth | grep -v grep | awk '{ print $2 }' | sudo xargs renice -20

sudo renice -20 578