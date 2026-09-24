---
title: Make the multiple-factory discount cumulative, as in Red Alert 2
category: balance
release: 0.2.0
breaking: true
migration:
- Rules that set `MultipleFactory` above `0`, or leave it unset, now get the new result. Set it to the build-time multiplier each extra factory should apply, such as `0.8` for 20% shorter builds per factory.
targets:
- type: key
  id: MultipleFactory
  effect: changed
- type: key
  id: MultipleFactoryCap
  effect: added
- type: system
  id: production
  effect: changed
credit: [ZivDero, CCHyper, dkeeton, Krnyoshi]
---

Each factory of a category past the first now multiplies that category's build times by `MultipleFactory` in `[General]` of `rules.ini`: at `0.8`, two factories build in 80% of the time and three in 64%. Before, the build time was multiplied once by `1 / ((factories - 1) × MultipleFactory)`, so at `1` a second factory gave no discount and below `1` it slowed production. The stock rules set `MultipleFactory=0`, which still gives no discount.

`MultipleFactoryCap` in the same section limits how many factories count toward the discount.

CCHyper is credited for the Vinifera version this follows, dkeeton for the ts-patches port of the Red Alert 2 formula, and Krnyoshi for the fix to Vinifera's cap.
