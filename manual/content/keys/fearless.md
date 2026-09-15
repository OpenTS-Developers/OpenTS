---
key: Fearless
summary: Keeps the soldier's fear at zero.
see_also: [Fraidycat, Doggie, "system:veterancy"]
when_omitted:
  kind: value
  value: "no"
---

Fear is a figure from `0` to `255` that a soldier accumulates as it is shot at. The ordinary paths that raise it are closed to a fearless soldier. A hit that arrives while its fear is still below `100` would jump it to that figure; a further hit would add up to `10`; and the [Panic](/mapping/missions/tmission-panic/) team mission drives a member straight to `255`. The `FEARLESS` [veteran ability](/systems/veterancy/#abilities) closes the same three.

With the figure held at zero, none of the behavior it drives ever starts. Such a soldier never drops prone because it is frightened, never scatters out of fear, and never has to work its nerve back.

:::caution[The two ways back down are skipped as well]
Fear is worked off at one point per logic frame, and the [Unpanic](/mapping/missions/tmission-unpanic/) team mission clears it outright. Both are skipped for a `Fearless=yes` type, and neither reads the veteran ability. That normally matters to nothing, because nothing raised the figure in the first place. The exception is the [`Fraidycat=yes`](/keys/fraidycat/) panic path, which ignores this flag when it sets fear. A type declaring both is frightened to `200` by the first hit it takes and has no way down again.
:::
