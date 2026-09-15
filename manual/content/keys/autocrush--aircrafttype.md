---
key: AutoCrush
scope: aircrafttype
label: Per-type automatic crushing
no_effect: true
see_also: ["Crusher", "PlayerAutoCrush"]
when_omitted:
  kind: value
  value: "no"
---

The flag is meant to let a human player's vehicle run an enemy over on its own initiative rather than firing at it, alongside the rules-wide [`PlayerAutoCrush`](/keys/playerautocrush/).

Both of its readers sit inside branches that have already established the answer, so neither can turn on what the flag was for. The routine that decides whether to crush an antagonist takes the flag into account only for a house a human is playing. The one place that routine is called from is reached only by a house no human is playing. The other read is an approach test that skips firing in favor of driving over the target. It is inside a computer-only branch as well, and there the flag is one half of an alternative whose other half is already true.

Crushing itself is unaffected. A [`Crusher=yes`](/keys/crusher/) vehicle ordered onto something crushable still runs it over, and a computer house still hunts for things to run over once its [`IQ`](/keys/iq/) reaches the [rules-wide threshold](/keys/autocrush/#scope-global-rules). The stock harvester sets `AutoCrush=yes` and the Devil's Tongue sets `AutoCrush=no`, and neither makes any difference.
