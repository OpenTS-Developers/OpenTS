---
key: Production
summary: The intelligence level at which a computer house starts itself building rather than waiting to be started.
see_also: [IQ, MaxIQLevels, BuildConst]
when_omitted:
  kind: value
  value: "5"
---

A computer house whose [`IQ`](/keys/iq/) reaches this level marks itself as started, as base-building and as alerted, and keeps all three marks for the rest of the session.

Being started is what lets the house act on the choices it has already made. An unstarted house still works out which structure, vehicle, soldier and aircraft it wants next, but every factory it owns passes over that choice, the construction yard included, so nothing is ever put in hand. Being marked as base-building is what makes its [MCV deploy itself and hunt](/keys/buildconst/). The alerted mark reaches nothing that runs.

Each of the three has a trigger action of its own: [Production Begins...](/mapping/actions/taction-begin-production/), [Auto Base Building...](/mapping/actions/taction-base-building/) and [Autocreate Begins...](/mapping/actions/taction-autocreate/). Further routes set the first two marks without any action. An MCV deploying into a construction yard outside a campaign game sets them, as does a house passing from a player to the computer while it still holds one. The [Begin production](/mapping/missions/tmission-begin-production/) team mission sets the started mark for its team's house. This threshold is what covers a campaign house that is meant to build without any of that.

Outside a campaign game every computer house is set to [`MaxIQLevels`](/keys/maxiqlevels/), so the threshold is open there for any value at or below that ceiling.
