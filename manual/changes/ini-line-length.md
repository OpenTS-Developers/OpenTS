---
title: Read an INI line of any length
category: fix
release: 0.2.0
targets:
- type: format
  id: ini-syntax
  effect: changed
credit: [ZivDero, CCHyper, tomsons26]
---

A line longer than 511 characters used to lose its tail without any record of it. The lists that hold the longest values, such as `Prerequisite`, `Explosion`, `DebrisTypes`, `Owner` and the voice lists in `rules.ini`, were cut again at 128. Every line is now read whole, and those lists reach their readers whole. A reader that still copies into fixed storage keeps as much as fits and writes the cut to the debug log once per key. A file with such a line now saves, and digests, with the whole line. A scenario whose digest an earlier build wrote still loads. A `ColorList` written without parentheses now reads as written, and a `[Tubes]` entry short of its five fields no longer crashes.

CCHyper and tomsons26 are credited for the Vinifera fix that widened the `Owner` buffer.
