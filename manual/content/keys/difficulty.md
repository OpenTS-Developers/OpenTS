---
key: Difficulty
summary: The campaign difficulty as a slider position, 0 for Easy, 1 for Normal and 2 for Hard.
see_also: ["system:difficulty"]
when_omitted:
  kind: value
  value: "1"
---

The difficulty slider on the campaign selection screen and the one in the game controls dialog both write this position. Both offer the three settings only, and the game controls one takes a new position only while no game is running. The read holds the figure to that same range. A value outside it is pulled back to the nearer end, and the corrected figure is written back to `sun.ini` with the rest of the options.

The value is used at the start of each campaign mission. It becomes the [difficulty slot](/systems/difficulty/#from-the-setting-to-a-slot) of every house under the player's control, and `2` minus it becomes the slot of every other house. A scenario outside a campaign ignores it and takes its difficulty from the session instead.
