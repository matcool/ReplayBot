from gd import memory
from pathlib import Path
memory.get_state().inject_dll(Path(__file__).absolute().parent / 'Debug' / 'ReplayBot.dll')