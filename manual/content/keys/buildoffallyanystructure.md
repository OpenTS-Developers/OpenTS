---
key: BuildOffAllyAnyStructure
summary: Which of an ally's buildings can anchor a placement when the match allows building off allies.
see_also: [BaseNormal, ConstructionYard, "system:base-adjacency"]
when_omitted:
  kind: value
  value: "yes"
---

```ini title="rules.ini"
[MultiplayerDefaults]
BuildOffAllyAnyStructure=no
```

With this assignment, only an ally's construction yards anchor a placement. The default lets any of the ally's buildings anchor one, on the same [`BaseNormal`](/keys/basenormal/) test that decides which of your own buildings do.

It is rules data rather than a match option, so every machine takes it from its own copy of the rules. It narrows the ally case alone: your own buildings anchor your placements either way, and the match must admit [building off an ally](/systems/base-adjacency/#building-off-an-ally) before the key decides anything.
