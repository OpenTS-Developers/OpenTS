---
key: FinalMovie
summary: The movie that plays once a campaign's closing mission is won.
see_also: ["Scenario", "Description", "CD", "EndOfGame"]
when_omitted:
  kind: value
  value: none
---

The movie plays when a won mission sets [`EndOfGame=yes`](/keys/endofgame/), just before the credits roll. The value is matched, without regard to letter case, against the movie names registered by the art layer's `[Movies]` list. That list is a registry a deployment can add to rather than a fixed engine table, so a custom film plays once its name is registered there. The file the engine then opens is the registered name with `.VQA` appended ([registering a movie](/formats/vqa/#registering-a-movie)).

A name the list does not hold, `<none>` included, sets nothing: the campaign keeps the closing movie its last read left in place. That is none at all unless an earlier battle file naming the same campaign set one. The campaign then goes from the mission's after-mission movies straight to the credits with no closing film.
