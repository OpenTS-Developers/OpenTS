---
key: UseNormalLight
summary: Draws the animation at full brightness instead of at the light level where it stands.
see_also: ["ShouldUseCellDrawer", "AltPalette"]
when_omitted:
  kind: value
  value: "no"
---

With the flag off, the animation is drawn at the brightness of whatever it belongs to. A veinhole tendril and an animation belonging to the ground take the tile brightness of their cell; an ordinary animation takes the cell's brightness. Under an ion storm, in the dark or beside a light source, the animation dims and brightens along with the ground. An animation that has been handed a palette of its own takes the brightness handed with it. One run by a structure takes the structure's own brightness. A piece of tiberium debris takes the brightness of the cell it landed in. A burning victim is given no brightness at all and takes the neutral level: the brightness a cell has before anything darkens or brightens it.

With it on, none of that is read and the animation is drawn at the neutral level. An explosion, a muzzle flash or a fireball wants it, since those light the ground rather than being lit by it, and a dark cell would otherwise swallow them.

The flag changes brightness only. Which palette the animation is remapped through is a separate decision, made by [`ShouldUseCellDrawer`](/keys/shouldusecelldrawer/) and [`AltPalette`](/keys/altpalette/).

The copy of a structure's animation that the fog of war draws in its place makes the same test. The shadow drawn beneath a bouncing animation is drawn at the neutral level whatever the flag says.
