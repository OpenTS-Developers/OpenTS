---
key: GenericClick
summary: Sound acknowledging a repair or sell click on a structure, and a click on a dialog control.
see_also: [GenericBeep, ScoldSound, SellSound]
when_omitted:
  kind: value
  value: none
---

```ini title="rules.ini"
[AudioVisual]
GenericClick=BUTTON1 ; a sound ID registered in SOUND.INI
```

Two unrelated kinds of event play the sound.

## In the game world

Clicking the repair cursor on a structure plays it at that structure's position, whichever way the toggle went: on for a damaged structure, and off again. Turning repair on for a structure already at full strength is the exception: that request still turns repair on, but it takes [`ScoldSound`](/keys/scoldsound/) instead and puts up no wrench. Clicking the sell cursor on a structure with build-up artwork plays it at full volume rather than from the structure's position, again for both directions of the toggle.

Both are gated on the structure belonging to a player-controlled house, so the same actions taken by a computer house (auto-repair, an AI selling off a structure, a trigger action) are silent. The sidebar itself never plays it.

## In dialogs

The owner-drawn dialog controls play it without a position when the player operates them: pressing a button, ticking a check box, clicking a combo box or an item in its open drop-down list. Selecting an item in a list box plays it, and so does moving a track bar far enough to change its value. The mission restatement screen's "More" button plays it on the frame it first draws pressed.
