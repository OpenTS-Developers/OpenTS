---
title: Retire the -CD launch option
category: feature
release: 0.2.0
breaking: true
migration:
- Replace `-CD<path>` in a shortcut or launcher with `-DATADIR=<path>` when the path holds the game's data, or name the folder in the deployment's `OPENTS.INI` `SearchPaths`.
targets:
- type: command
  id: launch:cd-path
  effect: removed
credit: [ZivDero]
---

`-CD<path>` no longer adds a local file-search path; an argument beginning with
it is ignored like any other the game does not recognize. The game data
directory covers pointing the game at its data, and a deployment's own
`OPENTS.INI` names the folders that data is sorted into, so the option had
become a second, narrower way of saying either.

With it goes the last of its disc-era plumbing: the semicolon-separated list it
accepted, and the upper-casing its path could not escape while every other
directory option keeps the case it was written in.
