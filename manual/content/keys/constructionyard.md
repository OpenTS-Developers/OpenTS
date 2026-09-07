---
key: ConstructionYard
summary: A BuildingType with this flag animates while it produces, refuses ordinary undeploy orders, and starts a computer house's base when an MCV deploys into it.
see_also: ["system:ai-base-building"]
when_omitted:
  kind: value
  value: "no"
---

While such a building produces, it runs its pre-production and production animations, and it is not counted as a vehicle even though it undeploys, so it reserves base ground the way a permanent structure does. Undeploying is refused except outside a campaign game, for a human owner, with the MCV redeploy option on.

The flag does not make a type count towards its house's construction yards: that tally admits only the BuildingTypes named by [`BuildConst`](/keys/buildconst/). Deploying a non-human house's MCV into a type carrying this flag outside a campaign game is what [starts that house's base](/systems/ai-base-building/#where-the-plan-comes-from).
