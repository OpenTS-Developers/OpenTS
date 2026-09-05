---
title: Decode delta-compressed AUD files and refuse unknown codecs
category: fix
release: 0.2.0
targets:
- type: format
  id: aud
  effect: changed
credit: [ZivDero]
---

An `.AUD` file using the Westwood delta compression now decodes and plays. A file with a compression code the game does not know is refused and stays silent; both used to play as noise.
