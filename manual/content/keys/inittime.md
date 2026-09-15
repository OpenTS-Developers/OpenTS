---
key: InitTime
summary: Parsed number that the engine never uses.
no_effect: true
when_omitted:
  kind: value
  value: "10000"
---

The value is stored on the scenario as the map is opened, and nothing reads it afterward: no timer starts from it, no countdown is seeded with it, and no gameplay path reads it. Nearly every shipped mission sets `InitTime=10000` purely because the line is present in its file.
