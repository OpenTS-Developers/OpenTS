---
title: Offer a building only when a yard can produce it
category: fix
release: 0.2.0
targets:
- type: system
  id: production
  effect: changed
credit: [ZivDero, AlexB]
---

The sidebar now offers a structure only when the house owns a construction yard acting for one
of the countries in its `Owner=` list, whatever that list holds. The test ran only for a type
with a single owner, so a structure owned by several countries reached the sidebar of a house
whose yard acted for none of them and was greyed the moment the factory search refused it.

AlexB is credited for the ts-patches patch that first ran the yard test for every structure.
