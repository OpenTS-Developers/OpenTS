---
format_id: vqa
title: VQA video
summary: Stores the full-motion video the engine plays full screen and in the radar pane.
kind: binary
extensions:
  - .VQA
role: video
related:
  - { type: format, id: mix }
source_files:
  - code/audio/audiomovie.cpp
  - code/movie.cpp
  - code/movies.cpp
  - code/rules.cpp
  - code/vqa.cpp
  - code/vqalib/buffer_.cpp
  - code/vqalib/drawer.cpp
  - code/vqalib/loader.cpp
  - code/vqalib/task.cpp
  - code/vqalib/vqafile.h
---

A `.VQA` is an IFF container holding vector-quantized video and, optionally, an audio track. The engine plays one either full screen, interrupting the mission, or a frame at a time inside the radar pane while the mission carries on.

## Registering a movie

Movie names come from the `[Movies]` section of the art layer: `ART.INI` first, then `ARTFS.INI` where that file is present. Each value is one movie name, without an extension; `.VQA` is appended when the movie is played.

```ini title="art.ini"
[Movies]
00=INTRO
01=GDI_M02
```

The section is walked in the order the file lists it in, and an entry key serves only to fetch its own value; nothing is taken from the key itself. A value that is empty registers nothing, and a name that is already registered is passed over, the comparison ignoring case. Only the first thirty-one characters of a value are kept.

One lookup answers both questions asked of a movie name: whether the registry already holds it, and which registered movie a setting means. That lookup returns nothing for `<none>` without searching at all, and both of its uses inherit that. Registration therefore never finds a `<none>` already present, and adds every one it meets in either file. A setting naming `<none>` selects no movie and is left at the value it already had. A name that was never registered is left at that value too.

The startup sequence, the score screen and the mission screens name a movie file outright instead of going through the list. Those names have the `.VQA` extension, and the section has no bearing on them.

## Finding the file

Playing a movie tests that the file exists, through the general file layer that searches both the mounted archives and the game directory. The movie is then opened again through the archive reader, which finds the member, opens the archive holding it, and reads the movie out of it in place. The general reader stands in for that open in one main menu sequence and nowhere else, so a loose `.VQA` in the game directory passes the existence test and then fails to open. A movie has to be an archive member.

Nothing is announced when a movie does not play, whatever the reason. A full-screen movie is shown only while all of these hold, tested in this order:

- the session is a campaign game, or the launch file asked for movies (without that, a skirmish does not qualify, and neither does any multiplayer kind);
- the file exists, as the general file layer sees it;
- the movie opened;
- its frame is not both narrower than 320 and shorter than 200. A movie that is gets opened and then discarded rather than shown.

A movie played into the radar pane is held to the session, file and open tests and not to the size test. Nothing is exempt from the session test: the startup and menu movies make the same test and pass it, because the session type stands at its campaign value while the menus are up. [Multiplayer movies](/systems/multiplayer-movies/) owns what the launch file's key changes, and how a movie ends once it has.

A movie that does play is centered on the surface it is drawn to unless the caller names a destination. Both the part of the frame taken and the area it lands in are then clipped, to the movie's own frame and to that surface.

## Container structure

Each chunk begins with a four-character identifier and a length, and the length is stored most significant byte first. A chunk of odd length is followed by a padding byte, so every chunk begins on an even boundary.

A file is taken for a movie only while all of these hold, tested in this order:

- its first chunk is a `FORM`;
- that chunk's length is not zero;
- the four characters recorded inside it are `WVQA`.

Anything else is refused as not a movie. The reader then works through the chunks that precede the frames, in whatever order the file presents them. Each kind below is set against what reading it does; the one to watch is the last, because it is the only kind that brings the pass to an end.

- `VQHD` supplies the header described below. A `VQHD` chunk whose length is not exactly that of the header record is refused the same way a bad container is.
- `LINF`, `CINF` and `PINF` are groups holding the loop, codebook and palette tables. `MFCI` and `MSCI` hold a table apiece of the chunk types the player keeps in a ring of buffers. Each entry names a chunk identifier and the size of one buffered entry, and an `MFCI` entry holds the period at which its chunk type recurs as well. Each of the five must open with its own header sub-chunk (`LINH`, `CINH`, `PINH`, `MFCH`, `MSCH`); one that opens with anything else refuses the file. `CLIP` is read in one piece instead, into a clipping rectangle that nothing afterwards reads. What `MFCI` and `MSCI` buffer is handed back out as each frame is drawn, and the game recognizes none of it, so those chunks are read and buffered to no end.
- Anything else is stepped over by its own length.
- `FINF`, the frame table, ends the pass, and reaching it is what allocates the playback buffers. Nothing else stops the pass, so a file with no frame table goes on consuming chunk headers until some read or seek fails and takes the movie with it.

The frames themselves follow as `VQFR`, `VQFK` and `VQFL` containers, each holding the codebook, palette and vector-pointer chunks for one frame. A frame's sound is not inside its container: `SND0`, `SND1`, `SND2` and `SN2J` sit beside the frame containers as chunks of the file. A chunk inside a frame container that is none of the three kinds that container accepts stops the movie loading where it is found, rather than being stepped over the way an unknown chunk outside one is.

## What the header supplies

The header is a fixed-length record read in one operation. The table below sets each value the player acts on against what that value decides. A movie that comes out silent, mistimed, discarded or in the wrong colors can be traced to a row here rather than to anything in its frames.

| Value | Effect |
| --- | --- |
| Version | Below 2, the audio buffers are sized as though the track were 22050 hertz, eight bit and mono, while the track itself is played at whatever the audio fields say; below 3, the reader supplies the terminating byte that codebook, palette and vector-pointer chunks of that vintage do not have; from 3 up, the largest frame size is counted in 256-byte units rather than bytes |
| Flags | A header that does not declare an audio track turns audio playback off for that movie |
| Frames | The total frame count, which bounds both the stop frame and any loop range asked for |
| Image width and height | The frame size, which decides placement, clipping and the too-small discard |
| Frame rate | Supplies both the load rate and the draw rate, since the game asks for the file's own rate |
| Color mode | A movie declaring mode 1 or 4 is drawn through the display's high-color translation tables |

The record also holds the block dimensions, a single-color count, the codebook entry count and the number of frames per codebook. The rest of it is a drawing position, the largest frame and codebook sizes, an audio preload figure, and the sample rate, channel count and sample width of each of the two audio tracks.

Two entries in that list decide nothing. The drawing position is read only where a movie is placed by offset instead of being centered or given a destination, and nothing asks for that. The second audio track's three fields are read only where that track is selected in place of the first, and nothing selects it, so a movie with two tracks plays its first one.

:::caution[Movies in a cached archive do not play]
The offset the archive reader seeks to is measured from the start of the archive file when the archive is not cached, and from the start of the archive's data section when it is. Only the first is a position within the file it then opens, so a movie inside an archive that was cached at startup is read from the wrong place, fails the container check, and is passed over in silence. Among the numbered expansion archives, the `ECACHE` set is cached and the `EXPAND` set is not, and of the two patch archives `PCACHE.MIX` is cached while `PATCH.MIX` is not. [MIX archives](/formats/mix/) covers what caching does.
:::

:::danger[A long movie name overruns the buffer the filename is built in]
The filename is assembled in a fixed twenty-byte buffer. `.VQA` takes four of those bytes and the string terminator a fifth, so a registered name of fifteen characters fills the buffer exactly and a sixteenth character writes one byte past its end. The registry accepts names of up to thirty-one characters, and playing a movie registered at that length writes sixteen bytes over whatever follows the buffer. The names the game ships with are all eight characters or fewer.
:::

## Sound and picture

The picture follows the sound. The player hands the sound track to the audio engine one block at a time, asks how much of it has been heard, and draws the frame due at that moment. The answer is the number of frames the mixer has taken from the track, less what the output device still holds, so the two stay in step through a device change or a stall on the game's own thread. While the sound stands still, as when the track ends before the picture does or the device is being recovered, the clock runs on wall time instead, and a pause is left out of the count altogether.

When the player falls behind and feeds a block twice, that block is taken off the count as well, so the picture waits for the sound rather than running ahead of it. A movie with no audio device is timed from the wall clock throughout.

A `SND1` chunk uses the Westwood delta compression and goes through the same checked decoder as an [AUD](/formats/aud/) file. A chunk that does not decode to its stated size plays as silence instead of being read past its end.
