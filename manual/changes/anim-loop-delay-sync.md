---
title: Draw animation loop delays from the synchronized generator
category: fix
release: 0.1.0
targets: []
credit: [ZivDero, tomsons26]
---

An animation that waits a random delay between loops now draws that delay from the generator every machine in a networked game shares. It used the local generator, whose result differs from machine to machine. That delay is stored in saved games and folded into the checksum players compare. The first time such an animation looped, the machines disagreed and the game reported a desynchronization.
