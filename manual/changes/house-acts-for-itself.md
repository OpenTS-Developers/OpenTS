---
title: Let a house act for its own country
category: fix
release: 0.2.0
targets:
- type: key
  id: ActsLike
  effect: changed
credit: [ZivDero, AlexB]
---

A house now acts for its own country unless its own section in the map file's house list says otherwise. That section's `ActsLike=` accepts a country's identifier as well as its position. The default came from the first three letters of the country's identifier: `GDI`, `Nod` or nothing. A third country therefore acted for no country outside a campaign, where house records are never read, and its construction yard built nothing. A name written here was read as zero. A country named `GDI-Reserve` no longer inherits a country from its name, and a country that takes no part in the multiplayer contest acts for none.

AlexB is credited for the ts-patches patch that first gave every country its own index here.
