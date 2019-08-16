from collections import defaultdict
import os

import mido
import serial


ARDUINO_BAUD_RATE = 560800

serials = os.listdir('/dev/serial/by-id/')
print('\n'.join(serials) + '\n')
arduino_port = '/dev/serial/by-id/' + [s for s in serials if 'arduino' in s.lower()][0]

midi_inputs = mido.get_input_names()
print('\n'.join(midi_inputs) + '\n')
keyboard_port = [m for m in midi_inputs if 'Keystation' in m and m.endswith(':0')][0]

# Note indices for 88-key keyboard go from 21 to 108
key_on = defaultdict(bool)

with mido.open_input('Keystation 88:Keystation 88 MIDI 1 20:0') as midi_in:
    with serial.Serial(arduino_port, ARDUINO_BAUD_RATE) as arduino_serial:
        for msg in midi_in:
            if msg.type == 'note_on':
                key_on[msg.note] = msg.velocity > 0
                arduino_serial.flush()
                arduino_serial.write([msg.note, 1 if msg.velocity > 0 else 0])
            print(msg)
