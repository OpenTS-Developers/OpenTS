# Developer documentation

The developer guides are split by subject:

- [Building OpenTS](BUILDING.md) — supported toolchain, commands, outputs,
  build identity, and continuous integration.
- [Style](STYLE.md) — source formatting, naming, C++ use, and comments.
- [History](HISTORY.md) — source lineage and reconstruction history.
- [Rationale](RATIONALE.md) — reconstruction tools, recovered structure, and
  non-obvious implementation choices.
- [Project direction](DIRECTION.md) — long-term architecture.
- [The saved game format](SAVE-FORMAT.md) — the layout of a `.SAV` file: its
  header, listing fields, compressed content, and object records.
- [The replay file format](replay/REPLAY-FORMAT.md) — the `.cncr` format shared
  between engines, with its schema in [replay.proto](replay/replay.proto).
- [The replay format layer](replay/FORMAT-LAYER.md) — the code in `code/replay/`
  that writes and reads that format, and the choices it makes.
- [ts-patches in OpenTS](TS-PATCHES.md) — each CnCNet ts-patches patch and
  what OpenTS does in its place.

See [CONTRIBUTING.md](../CONTRIBUTING.md) for contribution and review rules.
Player and modder documentation is under [manual/](../manual/README.md). When a
guide already covers a subject, link to it instead of copying the same facts.
