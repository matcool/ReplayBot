# ReplayBot

A replay bot for Geometry Dash

Demo Video:

<a href="https://youtu.be/3fcRO_AswyU" target="_blank"><img src="https://i.ytimg.com/vi/3fcRO_AswyU/maxresdefault.jpg" width="640" height="360" /></a>

## Usage

Replays are always stored in memory, to load one from a file click the load button.

Once you have a replay in memory go to the level, press the play button and then the restart button.

To record a replay press the record button and restart the level. Recordings use the default fps, to change it type `fps (number)` into the console (you can't change the fps while recording). You can record with practice mode but ship and ufo can desync very easily.

Some sample replays are available on the `replays/` folder.

*For the wasureta replay you'll need to type `2phackfix` into the console*

## Converter

```bash
# from replaybot to plain text
python converter.py replaybot myreplay.replay txt myreplay.txt
# txt back to replaybot
python converter.py txt myreplay.txt replaybot myreplay.replay
```

## TODO
- support unicode save paths
- (?) fix the practice mode inconsistencies (holding between attempts doesn't get added to actions, fix ship and ufo momentum not getting properly saved (idk how to do this))
- (?) make it more consistent (sometimes it desyncs on duals)

## Credits
@AndreNIH for [CappuccinoSDK](https://github.com/AndreNIH/CappuccinoSDK)
and everyone in the GD Programming Discord for all the help
