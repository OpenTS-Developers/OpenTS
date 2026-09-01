---
format_id: sound-ini
title: SOUND.INI
summary: Registers sound IDs and maps each ID to an AUD sample with priority and volume settings.
kind: file
filenames:
  - SOUND.INI
  - SOUND01.INI
key_scopes:
  - file: sound01.ini
    section:
      kind: identifier
      source: sound
related:
  - { type: format, id: aud }
source_files:
  - code/init.cpp
  - code/voc.cpp
---

Startup reads `SOUND.INI` and `SOUND01.INI` into one database. Whichever files are present are read, the expansion's file over the base one. A sound both files name comes from `SOUND01.INI`, and one only `SOUND.INI` names is kept. Either file alone is enough, and initialization stops only when neither can be read. Whether Firestorm is installed does not decide which file is read.

`[SoundList]` values register sound IDs. Each ID names a section and an `.AUD` sample with the same base name.

```ini title="SOUND.INI"
[SoundList]
0=MYALERT

[MYALERT]
Priority=10
Volume=1.0
```

The whole sound list is discarded and rebuilt each time the file is read, and the entries are registered in the order the section lists them. An ID that is already registered is filled in again rather than added a second time, so naming the same sound twice leaves one sound rather than two.

A sound fetches its sample as it is registered, before its own section is consulted, and holds it for the rest of the session. [AUD audio](/formats/aud/) covers where that sample has to live: an `.AUD` that is not in an archive cached at startup leaves the sound registered under its ID and silent. Nothing reports the miss, and the sound's own section is read either way.
