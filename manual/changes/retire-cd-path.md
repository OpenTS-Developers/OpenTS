---
title: Retire the -CD launch option
category: feature
release: 0.2.0
breaking: true
migration:
- Replace `-CD<path>` with `-DATADIR=<path>` where the path holds the game's data.
- Replace it with `-USERDIR=<path>` where the path held files of your own that stood in for the game's. That directory is read before any other, so what it holds still answers first, and it is where the game writes.
- Name the folder in the deployment's `OPENTS.INI` `SearchPaths` where it is one of several the game should always search.
targets:
- type: command
  id: launch:cd-path
  effect: removed
credit: [ZivDero]
---

`-CD<path>` no longer adds a local file-search path; an argument beginning with
it is ignored like any other the game does not recognize.

With it goes the last of its disc-era plumbing: the semicolon-separated list it
accepted, and the upper-casing its path could not escape while every other
directory option keeps the case it was written in.
