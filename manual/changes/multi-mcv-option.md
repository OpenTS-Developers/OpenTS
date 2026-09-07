---
title: Let any yard build for its owner with MultiMCV
category: feature
release: 0.2.0
targets:
- type: key
  id: MultiMCV
  effect: added
- type: system
  id: production
  effect: changed
credit: [ZivDero]
---

`[General] MultiMCV=yes` lets a construction yard produce for every country in a structure's
`Owner` list rather than only for the country it was built by, in the sidebar and in the factory
search alike. A rules set with one MCV per faction needs it once an MCV can come out of a
captured foreign factory. The key, its default of `no` and its meaning are Vinifera's, so a
rules set written for it carries over.
