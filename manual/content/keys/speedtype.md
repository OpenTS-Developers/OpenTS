---
key: SpeedType
summary: The terrain table column a vehicle's per-cell throttle and passability are read from.
see_also: ["Crusher", "MovementZone", "TrackedUphill", "WheeledUphill", "Speed"]
when_omitted:
  kind: context-dependent
  note: "`Track` in a `Crusher=yes` section and `Wheel` in every other."
---

One `rules.ini` section per [land type](/reference/enums/land-type/), `[Clear]`, `[Road]`, `[Water]` and the rest, has one figure per [speed type](/reference/enums/speed-type/) token. The figure is a decimal fraction of full speed, capped at 1.0, so `1` crosses at full speed and `0` refuses the cell outright. This key picks which of those figures the vehicle reads out of each of those sections.

```ini title="rules.ini"
[Water]
Track=0
Wheel=0
Hover=1

[MYSKIMMER] ; a UnitType registered in [VehicleTypes]
SpeedType=Hover
MovementZone=AmphibiousDestroyer
```

Two decisions read the chosen column. A cell whose figure is `0` is refused when the vehicle asks whether it may step into it. The refusal lapses when the question is asked at the height of a bridge deck over that cell, or with no height at all. A vehicle standing at ground level under a bridge is refused exactly as it would be in the open. The second decision belongs to the drive locomotor alone. A driven vehicle crosses every other cell at the fraction of full speed the figure gives, after the slope multiplier that the same value selects. `Track` takes [`TrackedUphill`](/keys/trackeduphill/) and [`TrackedDownhill`](/keys/trackeddownhill/); every other speed type, `Foot`, `Hover`, `Amphibious` and the rest alike, takes [`WheeledUphill`](/keys/wheeleduphill/) and [`WheeledDownhill`](/keys/wheeleddownhill/). A vehicle moved by any other locomotor reads its column for the per-step test and nothing else. A hovercraft or a tunneler therefore crosses ground priced at a tenth of full speed at the same speed it crosses ground priced at full. [Movement and terrain](/systems/movement-and-terrain/#what-each-locomotor-drives-its-speed-from) sets out where each of the ten takes its speed from.

## SpeedType and MovementZone

[`MovementZone`](/keys/movementzone/) answers a different question. Every cell is sorted once into one of a handful of terrain classes, and connected runs of cells sharing a class become the zones that reachability is judged against. A type's movement zone says which of those classes count as connected for it. That sorting reads the `Wheel` entry of a land type and no other, after `Water` and `Beach` cells have already been set aside as water. A vehicle's own `SpeedType` therefore never changes which cells the game treats as connected.

The two settings therefore have to agree. A hovercraft whose `[Water] Hover=` is above zero still gets no route across a lake unless its movement zone accepts water, because the zone map never read the `Hover` column. A movement zone that accepts water still strands it if its own column reads `0` for `Water`, because the per-step check refuses each cell as it is reached.

To keep a vehicle off one terrain, price its own column at `0` in that land's section. To confine it to a single land type whatever the other sections say, use [`MovementRestrictedTo`](/keys/movementrestrictedto/); that test runs ahead of this one and admits no exception from cost.

:::danger[An unrecognized value leaves the type with no speed type at all]
The value is matched against the token list and nothing else, and a spelling that misses resolves to no speed type rather than to a fallback. The repair that gives an unset speed type its `Track` or `Wheel` starting value runs before this key is read, so it cannot undo the bad value. Only a later rules layer that names the same section again gets another chance at it. Until then every throttle and passability question about the vehicle is answered from one slot short of the terrain table's first column. Those are the four bytes before the table's first entry, and for every land type but `Clear` they belong to the entry before it.
:::
