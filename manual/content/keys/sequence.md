---
key: Sequence
summary: The art.ini section that lays out the frames of every action an infantry type performs.
see_also: ["FireUp", "FireProne", "Crawls", "JumpJet", "Crew"]
when_omitted:
  kind: value
  value: none
---

The value names another section of the same file. That section has one entry per action the soldier can perform. Each entry holds up to four fields: the first frame of the run, the number of frames in it, and the number of frames from one facing to the next. An optional fourth field is a compass point the soldier is turned to when an ordinary run ends.

```ini title="art.ini"
[E1] ; the Image ID of the stock Light Infantry
Sequence=E1Sequence

[E1Sequence]
Ready=0,1,1      ; one frame per facing, frames 0-7
Walk=8,6,6       ; six frames per facing, frames 8-55
Idle1=56,15,0,W  ; one run of 15 shared by every facing, ending facing west
Prone=86,1,6     ; the first frame of each crawl run
Crawl=86,6,6
```

The frame drawn is the run's first frame, plus the animation stage taken as a remainder against the frame count, plus the soldier's heading reduced to one of eight blocks times the per-facing jump. A jump of `0` leaves the run facing-independent, as the `Idle1` entry above is. A [`JumpJet=yes`](/keys/jumpjet/) soldier with a target uses the direction to that target in place of its own heading.

An entry the named section does not have leaves that action with a frame count of `0`. An action whose frame count is `0` is never started at all, so a soldier whose section omits `Idle1` simply never plays it.

## Entry names

The table names every entry a sequence section may have and the action each one drives. Not every name is one the engine ever starts, and the line below the table says which four.

| Entry | Action |
| --- | --- |
| `Ready` | Standing at rest |
| `Guard` | Standing on guard |
| `Prone` | Lying prone at rest |
| `Walk` | Walking upright |
| `FireUp` | Firing upright |
| `Down` | Lying down |
| `Crawl` | Moving while prone |
| `Up` | Getting up |
| `FireProne` | Firing while prone |
| `Idle1`, `Idle2` | The two idle animations |
| `Die1` to `Die5` | The five death animations, chosen by the warhead that killed the soldier |
| `Hover` | Hovering on jump jets |
| `Fly` | Flying on jump jets |
| `Tumble` | Tumbling on jump jets |
| `FireFly` | Firing while flying |
| `Struggle` | Struggling inside a web |

Four of these are never started by anything: `Guard`, `Die3`, `Die4` and `Tumble`. Frames cut for them are never drawn.

:::danger[An infantry type with no sequence brings the game down]
Omitting the key leaves the type with no animation table at all, and the per-frame animation update reads that table without checking it, so the first update of any instance of such a type crashes. Naming a section the file does not contain is a different failure: every action is then left with a frame count of `0`, so the soldier starts no action and stands on the first frame of its artwork. A type registered only by being named elsewhere (the case [`Crew`](/keys/crew/) describes) has no section of its own and so falls into the first of these.
:::
