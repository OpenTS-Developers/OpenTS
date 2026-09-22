---
key: SimulateWhileUnfocused
summary: Keeps a campaign or skirmish running while another program has the focus.
see_also: [ToolTips, AltToRally]
when_omitted:
  kind: value
  value: "no"
---

By default, a campaign or skirmish pauses when its window loses the focus and resumes when the window gets the focus back. `SimulateWhileUnfocused=yes` keeps it running instead.

A game running without the focus plays no sound, takes no input and does not redraw the view. The simulation continues: production finishes, harvesters keep working and computer players keep playing.

The setting does not affect network games, which always keep running without the focus.

The game reads the setting only at startup.
