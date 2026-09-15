---
key: WaterCaves
summary: The tile set that supplies the eight cave mouths cut into a cliff at the water line.
see_also: [WaterfallEast, WaterSet]
when_omitted:
  kind: value
  value: "-1"
  note: No tile set is bound to the role.
---

One test reads the set, and it counts every one of the eight pieces as rock face with no subtile spared. Its callers are all [random map generator](/formats/map-seed/) terrain passes, among them the fill that squares off high ground and the pass that picks a variant for a tile it is about to lay. The role therefore bears on what the generator builds, and on nothing a scenario does afterwards. A cave mouth placed by hand takes whatever land type its own artwork reports, and this key changes none of that.

The test is guarded against a role that no tile set answered, so a theater with no cave mouths simply has none to offer it.
