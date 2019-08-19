from collections import defaultdict
import os

import mido
import serial


ARDUINO_BAUD_RATE = 9600

serials = os.listdir('/dev/serial/by-id/')
print('\n'.join(serials) + '\n')
arduino_port = '/dev/serial/by-id/' + [s for s in serials if 'arduino' in s.lower()][0]

midi_inputs = mido.get_input_names()
print('\n'.join(midi_inputs) + '\n')
keyboard_port = [
    m for m in midi_inputs
    if any(name in m for name in ('Keystation', 'Roland', 'Portable Grand'))
    and m.endswith(':0')
][0]

# Note indices for 88-key keyboard go from 21 to 108
key_on = defaultdict(bool)

notes = []

with mido.open_input(keyboard_port) as midi_in:
    with serial.Serial(arduino_port, ARDUINO_BAUD_RATE) as arduino_serial:
        while True:
            msg = midi_in.poll()
            if msg:
                if msg.type == 'note_on':
                    key_on[msg.note] = msg.velocity > 0
                    notes.extend([msg.note, 1 if msg.velocity > 0 else 0])
                elif msg.type == 'note_off':
                    key_on[msg.note] = False
                    notes.extend([msg.note, 0])
                print(msg)
            if arduino_serial.in_waiting > 0:
                bytes = arduino_serial.read(arduino_serial.in_waiting)
                print(bytes)
                print(notes)
                arduino_serial.write([int(len(notes) / 2)] + notes)
                arduino_serial.flush()
                notes = []
