---
title: Buildings that produce cash
summary: "Pays a structure's owner a fixed sum at a fixed interval, or takes it when the sum is negative, and pays a bonus to a house that captures the structure from a neutral house."
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

A BuildingType can pay its owner credits without harvesting anything. It has two independent payments, and a type may set either one or both:

- The recurring payment: [`ProduceCashAmount`](/keys/producecashamount/) credits every [`ProduceCashDelay`](/keys/producecashdelay/) frames, paid to whichever house owns the structure.
- The startup bonus: [`ProduceCashStartup`](/keys/producecashstartup/) credits, paid to the house that captures the structure from a neutral house.

On this page, a neutral house is one whose country sets [`MultiplayPassive=yes`](/keys/multiplaypassive/).

```ini title="rules.ini"
[OILDERRICK]                   ; an example BuildingType
Capturable=yes
ProduceCashStartup=1000        ; credits paid on capture from a neutral house
ProduceCashStartupOneTime=yes  ; pay that bonus only on the first such capture
ProduceCashAmount=100          ; credits paid each interval; a negative amount is taken instead
ProduceCashDelay=750           ; frames between payments
ProduceCashBudget=5000         ; total the recurring payment can move
ProduceCashResetOnCapture=no   ; yes gives each new owner a fresh budget
```

This structure pays 1,000 credits the first time a house captures it from a neutral house. From then on it pays its owner 100 credits every 750 frames, and stops after 50 payments have used up its 5,000-credit budget.

Payments go straight to the house's credits. They do not pass through a refinery, do not count as harvested, and are not limited by [`Storage`](/keys/storage/), so a house at its silo limit still receives every credit.

## The interval

A structure starts producing when it opens for business. A structure opens:

- when its construction finishes;
- when a structure the map places is revealed. For a computer-owned structure, or in any game other than a campaign mission, that happens when the structure is placed;
- again on every capture.

A structure first opens at the same moment its animations start and its other special abilities switch on.

In a campaign mission, a structure the player owns that starts under shroud produces nothing until the shroud over it lifts.

When the structure opens, it waits `ProduceCashDelay` frames and makes its first payment. It then waits the same delay before each later payment. Each structure keeps its own count, so two identical structures built a few seconds apart pay a few seconds apart.

The structure makes no recurring payment while any of the following holds:

- `ProduceCashDelay` is zero or negative;
- `ProduceCashAmount` is zero;
- the structure is being sold, counted from the moment the sale is ordered;
- its owner is a neutral house, so a neutral structure earns nothing until someone captures it;
- it has spent its [budget](#the-budget);
- its type is `Powered=yes` and it lacks [power](#power).

### Where the money goes

A positive `ProduceCashAmount` adds that many credits to the owner. A negative amount takes that many from the owner. Credits are taken first. If they run out, the house's stored Tiberium is sold at its credit value to cover the rest.

The owner never goes into debt. A house with no credits and no Tiberium left loses nothing.

:::caution[A drain counts as spending]
Money that a negative `ProduceCashAmount` takes counts as credits the house spent. The [multiplayer score screen](/systems/multiplayer-score-screen/) rates a house's economy by what it spent, so a structure that drains its owner raises that house's economy rating.
:::

## The budget

[`ProduceCashBudget`](/keys/producecashbudget/) caps the total that one structure's recurring payment can move. A budget of zero or less sets no limit.

Payments and drains both count against the budget by their size, so the budget limits a drain the same way it limits a payment. The budget counts the full amount due each interval, even when a drain finds the house with nothing left to take. The startup bonus does not count against it.

The final installment is reduced to what remains of the budget. For example, `ProduceCashBudget=250` with `ProduceCashAmount=100` pays 100, 100 and then 50.

A structure that has spent its budget stops producing and stays on the map.

A structure receives its budget when it first opens. [`ProduceCashResetOnCapture=yes`](/keys/producecashresetoncapture/) refills it on every capture, which revives a structure that had spent it. The refill is not limited to captures from a neutral house, so two opposing players who keep recapturing a spent structure refill it each time.

## Power

The power test applies only to a type that sets [`Powered=yes`](/keys/powered/). Such a structure produces nothing while it is switched off, stunned by an [EM pulse](/systems/emp-pulse/), or its house is short of [power](/systems/power/). A `Powered=no` structure keeps producing in all three cases.

While a `Powered=yes` structure is stopped this way, its interval pauses. When it can produce again, the interval resumes with the frames it had left.

:::note[A shortage stops cash structures that draw no power]
A power shortage stops cash production on a `Powered=yes` structure even when that structure draws no power itself. [Power](/systems/power/) explains which other powered functions a shortage stops, and on which structures.
:::

## Capture

[Engineers and capture](/systems/capture/) owns everything else that changing hands does to a structure. For cash production, a capture does three things:

- The interval restarts from the full `ProduceCashDelay` for the new owner, whether or not `ProduceCashResetOnCapture` refills the budget.
- The budget is refilled when `ProduceCashResetOnCapture=yes`, and otherwise carries over unchanged.
- The startup bonus may be paid, under the conditions below. Capturing a cash structure from another player transfers its recurring payment and pays no bonus.

The capturing house receives the startup bonus only when all of the following hold:

1. the house the structure came from is neutral;
2. `ProduceCashStartup` is above zero;
3. the capturing house is not itself neutral;
4. if the type sets [`ProduceCashStartupOneTime=yes`](/keys/producecashstartuponetime/), this structure has never paid the bonus before.

Without `ProduceCashStartupOneTime=yes`, a structure pays the bonus again each time it is recaptured from a neutral house.

:::caution[The one-time bonus and the budget are tracked per structure, not per type or per house]
A structure that [undeploys](/keys/undeploysinto/) and is deployed again counts as a new structure. It can pay the startup bonus again. It also starts with a full budget, however much of its budget the structure it replaced had used.
:::
