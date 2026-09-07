---
title: Read the side roster on the network game paths
category: fix
release: 0.2.0
targets: []
credit: [ZivDero]
---

The network and internet game paths now read the rules' `[Houses]` and `[Sides]` lists before
the countries, as the skirmish and client-launched paths already did. They re-read the countries
alone, so on those paths the side list was built from whichever country happened to name a side
first, and which side held which position depended on the order the countries were written in
rather than on the order the rules list them.
