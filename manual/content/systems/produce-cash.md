---
title: Buildings that produce cash
summary: "Pays a structure's owner a set sum on a set interval, drains one instead when the sum is negative, and hands a bonus to whoever captures the structure off a neutral house."
category: buildings-economy
keys:
  - Capturable
  - MultiplayPassive
  - Powered
  - ProduceCashAmount
  - ProduceCashBudget
  - ProduceCashDelay
  - ProduceCashResetOnCapture
  - ProduceCashStartup
  - ProduceCashStartupOneTime
related:
  - type: system
    id: tiberium
  - type: system
    id: capture
  - type: system
    id: power
---

A BuildingType carrying these keys moves money without harvesting anything. It offers two payments and may declare either without the other: a **recurring** one that [`ProduceCashAmount`](/keys/producecashamount/) pays its current owner every [`ProduceCashDelay`](/keys/producecashdelay/) frames, and a one-off **startup bonus** that [`ProduceCashStartup`](/keys/producecashstartup/) pays whoever captures the structure off a house taking no part in the contest. An oil derrick paying a lump sum on capture and a trickle afterwards is both on one type.

```ini title="rules.ini"
[SOMEBUILDING]                ; a BuildingType
ProduceCashStartup=0          ; integer, credits paid to whoever captures it off a neutral house
ProduceCashStartupOneTime=no  ; boolean, pay that bonus only once for the life of the structure
ProduceCashAmount=0           ; integer, credits paid each interval; a negative figure is taken instead
ProduceCashDelay=0            ; integer, frames between payments; zero produces nothing
ProduceCashBudget=0           ; integer, total the structure will ever move; zero sets no limit
ProduceCashResetOnCapture=no  ; boolean, hand each new owner a fresh budget
```

Credits go straight to the house. Nothing here passes through a refinery, counts toward what the house has harvested, or is limited by [`Storage`](/keys/storage/) capacity, so a house at its silo limit still banks every credit produced.

## The interval

A structure produces only once it is **open for business** — the same state that starts its animations and lets it perform its other special abilities. It is reached when construction finishes, when a structure already standing at scenario start is first revealed, and again on every capture. A structure the player owns that begins a campaign mission under shroud therefore produces nothing until the shroud lifts.

Opening loads the interval with `ProduceCashDelay` and starts it counting. When it reaches zero the payment is made and the interval reloaded. Each structure keeps its own count, so two identical structures built a few seconds apart pay a few seconds apart.

Four things stop the recurring payment outright:

- `ProduceCashDelay` is zero or negative;
- `ProduceCashAmount` is zero;
- the structure is being sold, from the moment the deconstruction order is queued rather than from the moment it takes effect;
- its owner's country sets [`MultiplayPassive=yes`](/keys/multiplaypassive/), so a neutral structure earns nothing while nobody has taken it.

### Where the money goes

A positive `ProduceCashAmount` is added straight to the owner's credits. A negative one is spent instead: credits go first, then Tiberium is liquidated out of the house's own structures at its credit value to cover the rest. Credits never go below zero, and a house with neither left loses nothing.

:::caution[A drain counts against the house as spending]
Money taken by a negative `ProduceCashAmount` joins the house's running total of credits spent, one of the figures the [multiplayer score screen](/systems/multiplayer-score-screen/) rates a house's economy on. A structure that drains its owner therefore also flatters that rating.
:::

## The budget

[`ProduceCashBudget`](/keys/producecashbudget/) caps the total a single structure will ever move. It is counted without regard to sign, so it limits a drain exactly as it limits a payment, and it is spent only by the recurring payment — the startup bonus is outside it. Zero, the default, sets no limit at all.

The final installment is paid in full rather than skipped, so `ProduceCashBudget=250` with `ProduceCashAmount=100` pays 100, 100 and then 50. A structure that has spent its budget stops producing and keeps standing.

[`ProduceCashResetOnCapture=yes`](/keys/producecashresetoncapture/) hands each new owner a fresh budget, which revives a structure that had already spent one. It applies to every capture, not only to a capture off a neutral house, so two allied players passing a spent structure back and forth refill it each time.

## Power

The power test applies only to a type declaring [`Powered=yes`](/keys/powered/). Such a structure produces nothing while it is switched off, stunned by an [EM pulse](/systems/emp-pulse/), or short of [power](/systems/power/). A type left on the default `Powered=no` ignores all three and keeps producing.

The interval is **paused** rather than restarted: a structure with four frames left when the lights go out has four frames left when they come back, so restoring power never pays out immediately.

:::note[A cash structure need not drain power to be stopped by a shortage]
Whether an ordinary `Powered=yes` structure stops in a brownout depends on it also drawing power and being switchable. Cash production reads the house's power balance directly, so a `Powered=yes` structure that drains nothing stops as well.
:::

## Capture

[Engineers and capture](/systems/capture/) owns the rest of what changing hands does to a structure.

- The interval is reloaded and restarted for the new owner, whatever the type's other keys say.
- The budget is refilled when `ProduceCashResetOnCapture=yes`, and otherwise carries over untouched.
- The startup bonus is paid, but only when the house the structure came from is `MultiplayPassive=yes`. Capturing a rival player's cash structure transfers the production and pays no bonus.

The bonus goes to the capturing house unless that house is itself `MultiplayPassive=yes`. [`ProduceCashStartupOneTime=yes`](/keys/producecashstartuponetime/) limits it to the first such capture; without it, a structure recaptured off a neutral house pays again every time.

:::caution[The one-time bonus is remembered per structure, not per type or per house]
A structure that [undeploys](/keys/undeploysinto/) and is deployed again is a new structure. It has paid no bonus yet, and its budget is full again, whatever the structure it replaced had already produced.
:::
