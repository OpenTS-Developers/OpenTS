---
title: Retire Westwood Online
category: feature
release: 0.2.0
breaking: true
migration:
- Play over a network or in skirmish instead. A Westwood Online game can no longer be started, and the World Domination Tour it hosted can no longer be entered.
- Delete the `[WOnline]` section from `sun.ini`, along with `PreferredServer`, `Locale`, `StoreNick` and `LastNickSlot` from `[MultiPlayer]`, or leave them to be ignored.
- Rebind whatever key was set to Page User. The command is gone, and the `[Hotkey]` entry naming it is ignored.
targets:
- type: command
  id: PageUser
  effect: removed
- type: key
  id: AllowFind
  effect: removed
- type: key
  id: AllowPage
  effect: removed
- type: key
  id: LangFilter
  effect: removed
- type: key
  id: LobMusic
  effect: removed
- type: key
  id: ShowAll
  effect: removed
- type: key
  id: LastNickSlot
  effect: removed
- type: key
  id: Locale
  effect: removed
- type: key
  id: PreferredServer
  effect: removed
- type: key
  id: StoreNick
  effect: removed
credit: [ZivDero]
---

The Westwood Online client has been removed. The service it logged in to, chatted
through, listed games on and reported ladder results to has not answered for years, so
the login, lobby, ladder and paging screens are gone.

The Internet button remains on the main menu and in the multiplayer game-type dialog, but
it is disabled. So is the World Domination Tour button: the tour was reached over the same
service, and while its screens and its maps are still in the game, no tour server can be
asked for a campaign.

A network game is unaffected, and so is everything the game does with a match once it has
started. The game still builds its results at the end of a match, describing the same
fields it always did, but there is no longer a server to send them to.
