---
key: TiberiumExplosive
scope: global-rules
label: Exploding harvesters
see_also: ["system:tiberium", "Power", "Storage"]
when_omitted:
  kind: value
  value: "no"
---

A vehicle destroyed while carrying Tiberium adds a second blast to its death explosion. For each Tiberium type it carries, the amount carried is multiplied by that type's [`Power`](/keys/power/#scope-tiberium), and the blast's damage is the sum. The blast goes through [`C4Warhead`](/keys/c4warhead/) and is credited to the destroyed vehicle.

The blast is a [wide-area blast](/systems/warheads/#the-wide-area-blast) over the 5×5 square of cells centered on the vehicle's cell. That section gives each cell's share of the damage.

The blast needs the vehicle's death explosion, so a vehicle type with none never blasts, whatever it carries. The death explosion comes from the type's [`Explosion`](/keys/explosion/) list, or from its [`ScrapExplosion`](/keys/scrapexplosion/) list while [`ScrapMetal`](/keys/scrapmetal/) is on and that list is not empty. [Spilled harvester loads](/systems/destruction-and-debris/#spilled-harvester-loads) covers when a vehicle plays it.

The harvester truce skips the blast. A mission turns the truce on with [`HarvesterImmune=yes`](/keys/harvesterimmune/). In a skirmish or a game against other machines, the game's truce option sets it.
