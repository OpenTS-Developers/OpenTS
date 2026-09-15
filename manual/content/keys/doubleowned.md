---
key: DoubleOwned
summary: Opens the type to every country, outside campaign games only.
see_also: ["system:production"]
when_omitted:
  kind: value
  value: "no"
---

Every question about who owns the type is answered with all countries at once instead of with its [`Owner=`](/keys/owner/) list, but only while the session is not a campaign game. A campaign ignores the flag, and the list stands. Because the answer then names every country, the house can build the type from [any construction yard able to produce at all](/systems/production/#what-a-house-may-build), whatever country that yard acts as. The factory search likewise stops caring which country the factory belongs to.

The `Owner=` line itself is left untouched, so one section can restrict a type in a campaign and open it to everyone in skirmish and multiplayer games.
