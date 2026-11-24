---
key: IsFlamingGuy
summary: Runs the animation up to eight cells, toward water when some is near, and then plays it out where it stops.
see_also: ["RunningFrames", "FlamingInfantry", "SplashList"]
when_omitted:
  kind: value
  value: "no"
---

The animation plays a burning infantryman: it runs up to eight cells, toward water when there is some nearby, then plays a death sequence where it stops. Its movement sets which frame it shows, so [`Rate=`](/keys/rate/#scope-animtype) has no effect on it. A burning man that runs off a bridge deck falls to the ground below.

## Running

Each time the animation reaches a cell, it picks the next one:

1. It looks for water in a ten-by-ten block of cells around itself, inside the playable area. A water cell does not count if it, or its neighbor to the west or north, lies under a bridge. Water toward higher X or Y cell coordinates counts as three cells nearer than it is.
2. If it finds water, it steps one cell toward it: diagonally if it can, otherwise along the X axis, otherwise along the Y axis.
3. If it finds no water, or cannot enter any of those cells, it takes the first neighboring cell it can enter, checking the eight directions from a random start.

It cannot enter rock, tunnel or wall terrain, a cell with a wall, a cell occupied by a structure or vehicle, or a cell more than two height levels above it. On a bridge deck, only a structure or vehicle on the deck blocks it. It moves 18 leptons per game frame.

The run ends when any of these happens:

- it arrives at its eighth cell;
- it arrives at a water cell while no more than one height level above the ground;
- it has no cell it can enter;
- it lands from a fall.

While running, the animation shows the run cycle for its direction of travel, one of eight. Each cycle holds [`RunningFrames`](/keys/runningframes/) frames, and the animation advances one frame every three game frames.

## Ending

When the run ends, the animation skips to its death sequence and plays it at one frame per game frame. The death sequence starts two frames after the last run-cycle frame, so the frame between them is never shown. The last frame of the shape's first half is the last frame shown before the animation removes itself.

A burning man that lands from a fall stops where he lands. If he lands in water, the first animation in the rules' [`SplashList`](/keys/splashlist/) plays there too.

## Artwork

The shape file has two halves of equal length. The first half contains the eight run cycles, the unused frame, and the death sequence. The second half contains a matching shadow for every frame of the first, drawn darkened on the ground under the frame that is showing. No shadow is drawn while the animation falls. An animation whose shape cannot be found draws nothing and is removed as soon as its run ends.

:::danger[Set RunningFrames on every IsFlamingGuy animation]
An `IsFlamingGuy=yes` animation without [`RunningFrames=`](/keys/runningframes/) crashes the game on its first logic pass, as long as it has a cell to run to.
:::

:::caution[Burning infantry use the local player's colors]
The rules' [`FlamingInfantry`](/keys/flaminginfantry/) animation is drawn in the local player's color scheme, not in the colors of the infantry's owner, so each player in a multiplayer game sees it in their own color. After a saved game is loaded, every `IsFlamingGuy=yes` animation uses the local player's color scheme, however it was created.
:::
