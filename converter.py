from dataclasses import dataclass
from struct import pack, unpack
import sys
import os

@dataclass
class Action:
    x: float
    hold: bool
    player_2: bool

@dataclass
class Replay:
    fps: float
    actions: list

def slice_per(l, n):
    yield from (l[i:i + n] for i in range(0, len(l), n))

def parse_replaybot(data: bytes) -> Replay:
    fps = unpack('f', data[:4])[0]
    actions = []
    for action_data in slice_per(data[4:], 6):
        if len(action_data) != 6:
            print('wtf', action_data)
        else:
            x, hold, player_2 = unpack('fbb', action_data)
            actions.append(Action(x, bool(hold), bool(player_2)))
    return Replay(fps, actions)

def parse_zbot(data: bytes) -> Replay:
    delta, speed_hack = unpack('ff', data[:8])
    fps = 1 / delta / speed_hack
    actions = []
    for action_data in slice_per(data[8:], 6):
        if len(action_data) != 6:
            print('wtf', action_data)
        else:
            x, hold, player_1 = unpack('fbb', action_data)
            # why the fuck are they 0x30 and 0x31
            # 0x30 = '0'
            # 0x31 = '1'
            # good job fig
            actions.append(Action(x, hold == 0x31, player_1 != 0x31))
    return Replay(fps, actions)

def dump_replaybot(replay: Replay) -> bytearray:
    data = bytearray()
    data.extend(pack('f', replay.fps))
    for action in replay.actions:
        data.extend(pack('fbb', action.x, action.hold, action.player_2))
    return data

def dump_zbot(replay: Replay) -> bytearray:
    data = bytearray()
    delta = 1 / replay.fps
    speed_hack = 1 # lol
    data.extend(pack('ff', delta, speed_hack))
    for action in replay.actions:
        data.extend(pack('fbb', action.x, 0x31 if action.hold else 0x30, 0x31 if (not action.player_2) else 0x30))
    return data

def parse_txt(data: bytes) -> Replay:
    lines = data.decode().splitlines()
    fps = float(lines.pop(0))
    actions = []
    for line in lines:
        if line:
            x, hold, player_2 = line.split(' ')
            actions.append(Action(x=float(x), hold=hold == '1', player_2=player_2 == '1'))
    return Replay(fps, actions)

def dump_txt(replay: Replay) -> str:
    final = ''
    final += f'{replay.fps}\n'
    for action in replay.actions:
        final += f'{action.x} {int(action.hold)} {int(action.player_2)}\n'
    return final[:-1]

if len(sys.argv) != 4:
    print('''\
Usage: python converter.py (from) format (to)
format can be either zbot, replaybot or txt
Example:
  Converts from zBot to ReplayBot
  python converter.py "Sonic Wave.zbot" replaybot "Sonic Wave.replay"

  Converts from zBot to txt
  python converter.py "Tartarus.zbot" txt "tartarus.txt"
''')
    exit(1)

if os.path.exists(sys.argv[3]):
    print(f'{sys.argv[3]} already exists')
    exit(1)

FORMATS = {'txt', 'replaybot', 'zbot'}

from_format = os.path.splitext(sys.argv[1])[1][1:]

to = sys.argv[2]

if to not in FORMATS:
    print(f'format can only be {FORMATS}')
    exit(1)

with open(sys.argv[1], 'rb') as file:
    data = file.read()

if from_format == 'zbot':
    replay = parse_zbot(data)
elif from_format == 'txt':
    replay = parse_txt(data)
else:
    replay = parse_replaybot(data)

with open(sys.argv[3], 'wb') as file:
    if to == 'zbot':
        file.write(dump_zbot(replay))
    elif to == 'txt':
        file.write(dump_txt(replay).encode())
    elif to == 'replaybot':
        file.write(dump_replaybot(replay))