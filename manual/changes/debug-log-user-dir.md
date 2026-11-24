---
title: Write the debug log to the user directory
category: feature
release: 0.3.0
targets:
- type: command
  id: launch:user-directory
  effect: changed
credit: [DaliborFox]
---

With [`-USERDIR`](/using/command-line/#user-directory), the debug log and out-of-sync reports are now written to a `Debug` folder in the user directory instead of beside the executable. Without the option, they stay beside the executable. See [Debug logs and console](/using/debug-logging/).
