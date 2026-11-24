---
title: End an ion blast with the scenario it was fired in
category: fix
release: 0.3.0
targets:
- type: system
  id: superweapons
  effect: changed
credit:
- ZivDero
---

An ion blast still spreading when a scenario ended went on in the next scenario, or in a saved game loaded during it, at the same map position, and stopped the infantry and vehicles it passed there. An ion blast now ends with its scenario.
