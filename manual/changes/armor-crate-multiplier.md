---
title: Correct the armor crate multiplier
category: fix
release: 0.1.0
breaking: true
migration:
- Replace each nonzero `Armor` row's third field with its reciprocal if existing rules or maps intentionally rely on the previous result. For example, change `Armor=33,ARMOR,0.5` to `Armor=33,ARMOR,2` to preserve the same damage reduction.
- Replace an `Armor` value of `0` with the intended positive armor divisor. Zero has no reciprocal and now leaves ordinary damage calculation dividing by zero.
targets:
- type: system
  id: crates
  effect: changed
credit: [ZivDero, Iran]
---

An armor crate now multiplies each eligible object's armor multiplier by the `Armor` row's third field. A value of `2` halves ordinary incoming damage, while `0.5` doubles it. The collector and each object in the crate radius keep their existing one-use checks, so armor crates still do not stack.
