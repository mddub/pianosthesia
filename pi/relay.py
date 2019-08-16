import serial

import mido

ser = serial.Serial('/dev/tty.usbmodem1411', 9600)

with mido.open_input('USB Axiom 49 Port 1') as i:
    for message in i:
        # middle C is 60
        # note looks like:
        # note_on channel=0 note=57 velocity=78 time=0
        # sustain pedal looks like:
        # control_change channel=0 control=64 value=127 time=0
        if message.type == 'note_on':
            ser.write([message.note - 21, 1 if message.velocity > 0 else 0])
            ser.flushOutput()
        print message
