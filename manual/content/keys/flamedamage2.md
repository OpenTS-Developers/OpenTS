---
key: FlameDamage2
summary: The warhead every damaging animation delivers its damage through.
see_also: ["FlameDamage", "Damage", "C4Warhead"]
when_omitted:
  kind: value
  value: none
---

An animation whose [`Damage`](/keys/damage/#scope-animtype) is above zero adds that figure to a running total on each frame it advances. It spends the whole points of the total as a blast at its own center, carrying the fraction over. Every one of those blasts uses this warhead. An animation attached to a terrain object accrues five times as fast. A fire burning on a tree spends its damage five times as quickly as the same fire on open ground.

The one exception is the animation whose ID is exactly `INVISO`, which delivers its damage through [`C4Warhead`](/keys/c4warhead/) instead. A bouncing animation takes no part in this at all; it does its damage where it comes down.

With no warhead named, the blast is abandoned before anything is damaged, so every damaging animation in the game becomes harmless while its artwork still plays.
