---
title: Read an INI line of any length
category: fix
release: 0.2.0
targets:
- type: format
  id: ini-syntax
  effect: changed
credit: [ZivDero]
---

A line longer than 511 characters used to lose its tail without any record of it, and the
lists that carry the longest values, such as `Prerequisite`, `Explosion`, `DebrisTypes`,
`Owner` and the voice lists, were cut again at 128. Every line is now read whole and those
lists reach their readers whole; a reader that still copies into fixed storage keeps as much as
fits and writes the cut to the debug log once per key. A file carrying such a line now saves,
and digests, with the whole line, and a scenario whose digest an earlier build wrote still
loads. A color list written without parentheses now reads as written, and a `[Tubes]` entry
short of its five fields no longer crashes.
