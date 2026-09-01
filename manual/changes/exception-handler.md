---
title: Report a crash with a minidump and a readable report
category: feature
release: 0.1.0
breaking: true
migration:
- Remove `-XE` from any shortcut. A debugger attached to the process already takes a crash before this handler does.
targets:
- type: command
  id: launch:no-exception-trap
  effect: removed
- type: command
  id: launch:exception-test
  effect: added
credit: [ZivDero]
---

A crash now writes a minidump, a readable report, and the end of that run's debug log into a
folder of its own under an `Exceptions` folder beside the executable. Reporting a crash means
attaching that one folder; see [Crash reports](/using/crash-reports/) for what it holds.

Crashes that previously went unreported now produce one: a crash on any thread rather than the
main thread alone, a crash during startup before the window, sound, and renderer are up, a
stack overflow, a pure virtual call, a rejected runtime argument, an aborted run, and an
unrecoverable engine error, whose message previously went somewhere a windowed program had no
way of showing.
