---
format_id: mapsel-ini
title: MAPSEL.INI
summary: Lists each campaign house's stages in order, and the mission, map-screen presentation and choices each stage sets.
kind: file
filenames:
  - MAPSEL.INI
  - MAPSEL01.INI
source_files:
  - code/mschoice.cpp
  - code/mapsel.cpp
related:
  - type: system
    id: campaign-progression
  - type: format
    id: vqa
  - type: format
    id: aud
---

When a campaign mission ends and the campaign goes on, the mission played next is read out of this file. Both ways of advancing decide it there: picking a region on the map selection screen, and a mission naming its own successor. [Campaign progression and carry-over](/systems/campaign-progression/) owns that sequence and the state carried across it; this page owns the file the choice comes from.

## Which file is read

The base game reads `MAPSEL.INI`. A mission demanding an addon reads `MAPSEL` and that addon's two-digit number instead, so Firestorm's file is `MAPSEL01.INI`. The file is read afresh at every advance, and nothing from one read is kept for the next.

The section read is the one named for the playing house's INI name, which follows from the mission's [`Player=`](/keys/player/#scope-scenarios) setting, so a campaign played as GDI never reads the Nod section. A missing file, or a house section with no numbered entries, fails the read, and [the win sequence](/systems/campaign-progression/#the-win-sequence) owns what that does.

## House sections

A house section lists the stages of that house's campaign in order. The numbered keys `1` upward each name a stage section, and the list stops at the first number that is missing, so the entries have to run from `1` without gaps. At most 100 stages can be listed.

A stage's position in the list is its stage number. That number is what the engine stores in the scenario record, and what it uses to find the current stage again at the next advance.

Two further keys name the presentation material the map selection screen uses:

- `Anims=` names the section of looping map animations.
- `Sounds=` names the section of sound effects.

## Stage sections

Each stage has a section whose name is the label the house list uses. The section records what the stage plays and how the map screen presents it.

| Key | Value |
| --- | --- |
| `Scenario=` | The mission file the stage plays. This is the name the screen hands on, and the one [`NextScenario=`](/keys/nextscenario/) has to match. |
| `Description=` | Text printed while the cursor rests on the stage's region. A number is looked up in the game's string table; a value starting with a letter names a section of numbered lines in this file, read as one description. |
| `VoiceOver=` | An [AUD](/formats/aud/) sample streamed when the cursor comes to rest on the region. |
| `MapVQ=` | The [VQA](/formats/vqa/) movie the screen presents the map with. The screen is built around it, and a stage without one cannot be presented. |
| `Overlays=` | Up to two comma-separated shape names, faded in one after another over the movie's last frame. |
| `ClickMap=` | A 256-color PCX file, at the movie's resolution, whose pixel colors mark out the selectable regions. |
| `Targets=` | The count of target markers, then each marker's position in the 640 by 400 presentation area: `<count>,<x>,<y>,<x>,<y>`. The markers pair with the numbered entries in order. |
| `Text1=` to `Text7=` | Captions printed over the movie, each `<x>,<y>,<delay>,<text>`, the delay in game frames from the start of the movie. |

The numbered assignments of a stage section are its choices. Each value names the stage the choice leads to, and each key is the click-map pixel color that selects it: the screen reads the color under the cursor from the click map and matches it against these keys. A color with no entry selects nothing. Stage labels are matched without regard to case. The key numbers are colors, not an order, so they need not run without gaps.

The shipped GDI chain opens like this, and shows the two shapes a choice list takes:

```ini title="MAPSEL.INI"
[GDI]
Anims=Anims
Sounds=GDISFX
1=GDI01   ;1A
2=GDI02   ;2A
3=GDI03   ;3A1
4=GDI04   ;3A2

[GDI01]
Scenario=GDI1A.MAP
Description=768
VoiceOver=GDI-01.AUD
MapVQ=GDIMAP01.VQA
Overlays=RG02A.SHP,RN02A.SHP
Targets=1,180,80
ClickMap=GDICLK01.PCX
2=GDI02 ;2A

[GDI02]
Scenario=GDI2A.MAP
Description=769
VoiceOver=GDI-02.AUD
MapVQ=GDIMAP01.VQA
Overlays=RG03AB.SHP,RN03AB.SHP
Targets=2,290,88,218,108
ClickMap=GDICLK01.PCX
3=GDI04 ;3A2
4=GDI05 ;3B
```

`[GDI]` lists four stages, whose stage numbers are 1 to 4 in the order written. Stage `[GDI01]` plays `GDI1A.MAP` and offers one choice. Stage `[GDI02]` plays `GDI2A.MAP` and offers two, selected by the click-map colors 3 and 4. The fence leaves out the animation and sound sections the house section names; both are described below.

## How a stage is chosen

Both ways of advancing end at a stage of this file, never directly at a mission file.

With the map screen, the stage's movie plays, its overlays and target markers arrive, and the player picks one of the regions the click map colors. The picked region's entry names the next stage, and that stage's `Scenario=` value is the mission that loads.

Without the screen, when the mission sets [`SkipMapSelect=yes`](/keys/skipmapselect/), the name given by [`NextScenario=`](/keys/nextscenario/) or [`AltNextScenario=`](/keys/altnextscenario/) is compared, ignoring case, against the `Scenario=` value of each stage the current stage offers, and the first match is taken. Only stages the current stage lists can be reached this way.

## Animation and sound sections

The section named by `Anims=` carries the looping animations that decorate the map:

- `TextRect=` is `<x>,<y>,<width>,<height>`, the area stage descriptions are printed in, relative to the 640 by 400 presentation area. Left out, it is the whole area.
- `Palette=` names the palette the animations and target markers are drawn through. The screen refuses to start without it.
- The numbered keys `1` upward, with no gaps and at most 100 of them, each hold `<file>,<x>,<y>,<rate>`: a shape file, its position, and the delay between its frames in game frames.

The section named by `Sounds=` maps the screen's sound events to samples. Each key is the name of an event, and each value is `<file>,<volume>`, the volume a percentage clamped to the range `0` to `100`. The events looked up are `Overlay` as each overlay arrives, `TargetFlyIn` as each marker flies in, and `Click` when a region is picked. `EnterRegion` and `ExitRegion` play as the cursor crosses a region's boundary. `MouseOnMap` plays when it moves onto a region from the background, `MouseOffMap` when it moves back. An event the section does not set stays silent.
