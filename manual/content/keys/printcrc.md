---
key: PrintCRC
summary: The playback frame at which the game writes a synchronization dump and quits.
see_also: ["Frame", "Type", "CheckHeap"]
when_omitted:
  kind: value
  value: "2147483647"
---

The game reads the number from the `[SyncBug]` section of `sun.ini` as the multiplayer menu is entered, and only while recording playback is armed. Recording playback then compares the frame counter against it just after computing each frame's checksum. On reaching it the game writes the [out-of-sync report](/using/out-of-sync-reports/) into the `Debug` folder beside the executable and then exits. It is the same report a live network game writes when it detects a divergence.

```ini title="sun.ini"
[SyncBug]
PrintCRC=1500 ; report and exit at this frame
```

Playback is armed from the command line. A Debug build answers `-Y`. Any build answers `-ATTRACT`, which lets the classic main menu time out into playing a recording back.

:::caution[Armed playback with a readable recording never performs this read]
A readable `RECORD.BIN` makes an armed game skip the main menu and start the playback directly, so the multiplayer menu is never entered and the number stays at the `0` it was constructed with. The comparison succeeds on the very first frame, and the dump is written before the recording has played anything.
:::

A hand-written value therefore acts in exactly one flow: a Debug build launched with `-Y` and no readable `RECORD.BIN` present. The main menu then shows, entering the multiplayer menu performs the read, and the game that follows writes the report and exits once its frame count reaches the written number. A Release build has no such flow. Attract mode is the only thing that arms playback there, and it arms only when the recording that skips the read is present, so a Release build never reads a written `PrintCRC` at all.
