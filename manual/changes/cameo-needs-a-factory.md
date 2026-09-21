---
title: Only offer a cameo some structure can build
category: fix
release: 0.2.0
targets:
- type: system
  id: sidebar
  effect: changed
credit: [ZivDero]
---

A type that none of the player's factories may build, because its [`Owner=`](/keys/owner/) matches none of them, is now kept off the sidebar. It used to appear and disappear in a loop, replaying the new construction options announcement each time.
