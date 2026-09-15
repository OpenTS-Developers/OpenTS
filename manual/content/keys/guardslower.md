---
key: GuardSlower
summary: Weights the team's formation toward the members it counts as slow.
see_also: [Stray, TechLevel, DeploysInto, "system:ai-team-execution"]
when_omitted:
  kind: value
  value: "no"
---

In practice this setting is nearly inert. It pulls the team's center point toward its slow members, and that matters only when the averaged center cell is impassable. The lagging-member test never runs, and nothing reads the reinforcement record it maintains.

A member counts as slow when its type sets [`TechLevel=-1`](/keys/techlevel/#scope-aircrafttype) or names a [`DeploysInto`](/keys/deploysinto/) building: the unbuildable escorts and the vehicles that would rather set up than fight. The judgement is made afresh each time it is needed, and it is the only thing this setting keys off.

One thing then changes. As the team's center point is averaged, a slow member's position is added twice instead of once, so the average is pulled toward it. That average is usually thrown away again: [the center becomes the position of the member nearest the team's target](/systems/ai-team-execution/#the-teams-center) unless that member could not enter the averaged cell. The weighting therefore reaches the team's behavior only where the average survives (an averaged cell on a cliff, in water, or under a building) and changes nothing at all the rest of the time.

The setting is read in one further place, and that place never runs. The test that looks for a member lagging behind the formation allows a member that does not count as slow only a third of [`Stray`](/keys/stray/), so the fast members would be pulled back into formation sooner. [That test is unreachable](/systems/ai-team-execution/#settings-and-state-without-effect), so neither distance is ever measured.

:::caution[No member is ever slow for want of a weapon]
The judgement asks three questions, and the first, whether the object's primary weapon slot is empty, can never answer yes, because that lookup returns a slot rather than a weapon and so is never empty. An unarmed escort is counted as slow only if its type also happens to be unbuildable or to deploy into something.
:::

The setting maintains one further piece of state, a record of whether the team is above its under-strength threshold, and nothing reads it.
