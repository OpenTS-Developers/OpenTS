---
title: Compile Blowfish into the binary
category: internal
release: 0.1.0
targets: []
credit: [ZivDero, tomsons26]
---

OpenTS builds the Blowfish cipher into the executable instead of reaching it through the COM object in `blowfish.dll`. The cipher and the data it reads are unchanged, so encrypted archives, saves, and network packets keep their existing contents.

Startup no longer instantiates that object, and no longer falls back to loading the library and calling its self-registration entry point. A run directory therefore does not need `blowfish.dll`. A missing, unregistered, or damaged copy no longer ends startup with the COM preparation error.
