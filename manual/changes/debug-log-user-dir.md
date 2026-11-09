---
title: Store logs into the user directory when specified
category: feature
release: 0.2.0
targets:
- type: command
  id: launch:console-debug
  effect: changed
credit: [DaliborFox]
---

Previously, logs were always stored in the `Debug` folder beside the game executable.
Since it's quite likely for the game not to have write access to this folder, it now
stores the logs in a `Debug` folder in the user directory when the
[`-USERDIR`](/using/command-line/#user-directory) command-line option is provided.

See [Debug logs and console](/using/debug-logging/) for specific details.
