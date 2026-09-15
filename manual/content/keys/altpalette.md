---
key: AltPalette
summary: Draws the animation through the first declared color scheme instead of the shared animation palette.
see_also: ["ShouldUseCellDrawer", "UseNormalLight", "Tiled"]
when_omitted:
  kind: value
  value: "no"
---

The animation's pixels are remapped through the first color scheme the rules declare in `[Colors]`, in place of the shared palette every other animation is drawn with. It changes only the colors, not how brightly the animation is lit.

The choice of palette is made in order, and this flag is the last thing read. A veinhole tendril, an animation belonging to the ground it stands on, and an animation that has been handed a palette of its own are all settled before the flag is reached. None of them looks at the flag. A handed palette means a structure's animation taking its owner's house colors, a piece of tiberium debris, or a burning victim. In practice the flag reaches an animation standing on its own, or one a structure runs with [`ShouldUseCellDrawer=no`](/keys/shouldusecelldrawer/).

A [`Tiled=yes`](/keys/tiled/) animation is drawn through the shared palette on every copy, so the flag does nothing there. The stand-in a structure's animation leaves behind under the fog of war ignores it as well.
