---
key: Priority
scope: teamtype
label: Team recruitment priority
see_also: ["system:ai-team-production", Max, Recruiter]
when_omitted:
  kind: value
  value: "7"
---

An object already on a team is taken from it only when another team recruits, and only by a team whose value is strictly greater. Two TeamTypes sharing one value never poach from each other, and a team on the default `7` is safe from every other team left at the default. The value has no weight in [trigger selection](/systems/ai-team-production/#the-weighted-draw) and is not a build order.

It is read once more when a house's base comes under attack. Every team of that house whose value is below [`SuspendPriority`](/keys/suspendpriority/) in `[General]` has each of its members removed and is suspended for [`SuspendDelay`](/keys/suspenddelay/) minutes, freeing those members to answer the attack. The emptied team's logic does not run again until the timer expires. That threshold defaults to `20`, so a team left on the default `7` is stripped whenever the response runs.
