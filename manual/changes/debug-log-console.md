---
title: Write a debug log every run and open a debug console
category: feature
release: 0.1.0
targets:
- type: command
  id: launch:console-debug
  effect: changed
credit: [ZivDero]
---

Every run now writes a log to a `Debug` folder beside the executable and keeps two weeks of
them. Release builds wrote nothing at all before, so a crash left nothing to read; debug builds
appended to a single `DEBUG.TXT` that the next launch overwrote. That file is no longer
written, and an existing one is left alone.

`-XC` opens the debug console in a release build, where it previously was rejected outright and
stopped startup. See [Debug logs and console](/using/debug-logging/) for what a log records and
what to consider before sharing one.
