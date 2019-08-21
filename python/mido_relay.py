import os
import random
import time

import mido
import serial


ARDUINO_BAUD_RATE = 560800

# in seconds
AUTO_PLAY_DELAY_AFTER_SCRIPT_START = 5
AUTO_PLAY_DELAY_AFTER_LAST_INPUT = 600
AUTO_PLAY_DELAY_BETWEEN_TRACKS = 300

MIDI_TRACKS_DIR = 'tracks'


random.seed(time.time())


abs_midi_tracks_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), MIDI_TRACKS_DIR)
all_midi_choices = [
    os.path.join(abs_midi_tracks_dir, f)
    for f in sorted(os.listdir(abs_midi_tracks_dir))
    if f.endswith('.mid')
]
remaining_midi_choices = all_midi_choices.copy()
# start at a random point in the track order
remaining_midi_choices = remaining_midi_choices[-random.randint(1, len(remaining_midi_choices)):]
print(all_midi_choices)

def choose_next_track_file():
    global remaining_midi_choices
    if not remaining_midi_choices:
        remaining_midi_choices = all_midi_choices.copy()
    return remaining_midi_choices.pop(0)


next_track_start_time = time.time() + AUTO_PLAY_DELAY_AFTER_SCRIPT_START
track_note_on = {i: False for i in range(21, 109)}
track_messages = []

def get_next_midi_message(midi_in, midi_out):
    global next_track_start_time
    global track_messages
    global track_note_on
    global track_start
    global elapsed_track_time

    msg_in = midi_in.poll()

    if msg_in:
        next_track_start_time = time.time() + AUTO_PLAY_DELAY_AFTER_LAST_INPUT

        if track_messages:
            track_messages = []
            # volume
            midi_out.send(mido.Message('control_change', channel=0, control=7, value=100))
            # reset all controllers
            midi_out.send(mido.Message('control_change', channel=0, control=121, value=0))

        return msg_in

    elif time.time() < next_track_start_time and any(track_note_on.values()):
        # clear pixels that are still on after human input interrupted the track
        on_note = [note for note, on in track_note_on.items() if on][0]
        track_note_on[on_note] = False
        return mido.Message('note_off', note=on_note, velocity=0)

    elif track_messages:
        if time.time() - track_start > elapsed_track_time + track_messages[0].time:
            msg = track_messages.pop(0)
            if msg.type == 'note_on' and msg.velocity > 0:
                track_note_on[msg.note] = True
            elif msg.type == 'note_off' or (msg.type == 'note_on' and msg.velocity == 0):
                track_note_on[msg.note] = False
            elapsed_track_time += msg.time
            midi_out.send(msg)
            if not track_messages:
                next_track_start_time = time.time() + AUTO_PLAY_DELAY_BETWEEN_TRACKS
            return msg

    elif time.time() > next_track_start_time:
        infile = mido.MidiFile(choose_next_track_file())
        track_messages = [msg for msg in infile if not msg.is_meta]
        track_start = time.time()
        elapsed_track_time = 0

    return None


note_send_buffer = []

def loop(midi_in, midi_out, arduino_serial):
    global note_send_buffer
    msg = get_next_midi_message(midi_in, midi_out)
    if msg:
        if msg.type == 'note_on':
            note_send_buffer.extend([msg.note, 1 if msg.velocity > 0 else 0])
        elif msg.type == 'note_off':
            note_send_buffer.extend([msg.note, 0])
        print(msg)
    if arduino_serial.in_waiting > 0:
        bytes = arduino_serial.read(arduino_serial.in_waiting)
        arduino_serial.write([int(len(note_send_buffer) / 2)] + note_send_buffer)
        arduino_serial.flush()
        note_send_buffer = []


if __name__ == '__main__':
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

    print('sleeping while fluidsynth initializes...')
    time.sleep(10)
    midi_outputs = mido.get_output_names()
    print('\n'.join(midi_outputs) + '\n')
    fluidsynth_port = [m for m in midi_outputs if 'fluid' in m.lower()][0]

    with mido.open_input(keyboard_port) as midi_in:
        with mido.open_output(fluidsynth_port) as midi_out:
            with serial.Serial(arduino_port, ARDUINO_BAUD_RATE) as arduino_serial:
                arduino_serial.write([0])
                arduino_serial.flush()
                while True:
                    loop(midi_in, midi_out, arduino_serial)
