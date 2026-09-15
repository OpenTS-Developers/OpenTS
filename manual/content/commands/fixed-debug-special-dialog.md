---
command_id: fixed:debug-special-dialog
---

The control opens no dialog in any session type, for two separate reasons. The routine that would draw the special dialog is an empty body sitting inside a block the compiler never reaches, and the arm of the menu loop that would have called that routine is compiled out as well. A request for the dialog therefore survives with nothing left to answer it.

What the key does still depends on the session. In a campaign game it calls the in-game menu handler directly instead of recording a request. That handler does nothing unless a request is outstanding, so the key has no effect. Every other session type, skirmish included, takes the other branch: the key records the request and consumes the press, leaving the handler to be reached from the main loop in the ordinary way.

:::danger[Outside a campaign game the request hangs the scenario]
Every session that is not a campaign stores a request that no compiled arm answers. The handler pauses the scenario and then loops until the request is cleared. Every arm that survives compilation clears it, but this request reaches the default arm instead, which clears nothing. The loop therefore never ends and the scenario is never resumed. The only thing that prevents it is the player already being flagged to win, lose or die when the key is pressed: the handler then discards the request and returns before it pauses anything. The other test refuses a menu to a player still in the game, but the flag it reads is set at the start of every scenario, so it never runs.
:::

[Developer mode and diagnostics](/systems/developer-mode/) covers the flag that arms the keys handled directly in code.
