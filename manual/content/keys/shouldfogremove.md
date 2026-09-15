---
key: ShouldFogRemove
summary: Hides the animation while the ground it plays over is under the fog of war.
see_also: ["ActiveAnim", "system:map-visibility"]
when_omitted:
  kind: value
  value: "yes"
---

The flag hides the animation; nothing about it lifts fog or reveals ground.

Two separate paths apply it, one for each kind of animation. An animation standing on its own is skipped by the tactical pass while the cell under its center is fogged, so it cannot betray what the player is no longer allowed to see. An animation attached to a moving object, such as a fire attached to the object it burns, counts as standing on its own. The test looks at the cell under the animation and at whether a structure runs it, not at what it is attached to. An animation a structure runs is not covered by that test; it is hidden while the structure it belongs to is fogged. The structure and its animations are drawn from the stand-in the fog took of them instead, so the animation does not vanish; it freezes on the stage it had reached and thaws when the fog lifts.

With `no`, neither test applies: the animation keeps being drawn live over fogged ground, and a structure's animation keeps cycling on top of its own frozen stand-in. It suits an animation whose absence would look wrong, such as one belonging to a structure the player is already remembering. Used carelessly, it is also the setting that gives a hidden explosion away.

[Shroud, fog and the radar map](/systems/map-visibility/#the-fog-of-war) describes what the fog photographs and when it lifts.
