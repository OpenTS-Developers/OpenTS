---
key: PrintCRC
summary: Meant to name the playback frame at which the game writes an out-of-sync report and quits, though no launch honors a chosen frame.
see_also: ["Frame", "Type", "CheckHeap"]
when_omitted:
  kind: value
  value: "2147483647"
---

`PrintCRC` is meant to stop recording playback at a chosen frame, but no launch lets it do that. When the value does take effect, the game writes an [out-of-sync report](/using/out-of-sync-reports/) into the `Debug` folder beside the debug log and exits.

Only a Debug build started with [`-XY`](/using/command-line/#playback) plays a recording back, so a Release build never reads the value. In a Debug build started with `-XY`, the result depends on whether `RECORD.BIN` can be read:

1. With a readable `RECORD.BIN`, playback starts without the main menu, and the value is never read. The game writes the report and exits on frame 0, before the recording has played anything.
2. Without one, the main menu shows, and picking multiplayer play reads the value. The game that follows has no recording to play back, so it ends at the first frame that tries to read one, frame 1 in a skirmish game. Only a value no higher than that frame produces the report.
