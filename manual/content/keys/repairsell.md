---
key: RepairSell
summary: The house intelligence level at which the computer begins repairing its damaged buildings and selling the ones it cannot afford.
see_also: ["system:repair"]
when_omitted:
  kind: value
  value: "3"
---

The house's [`IQ`](/keys/iq/) is compared against this figure before anything else in [the computer's repair decision](/systems/repair/#when-the-computer-repairs), and it gates the sell-back branch on the same test. Computer houses outside campaign games hold the maximum intelligence level, so the gate is open for them whenever this is at or below [`MaxIQLevels`](/keys/maxiqlevels/); in a campaign the mission's own house entries decide it.

A house under player control skips the captured-or-flagged test that limits which buildings a computer repairs. Raising the player house's `IQ` to this level in a map therefore hands the player automatic, unprompted repair of every damaged building it owns, while money and the one-start-per-pass throttle still apply.
