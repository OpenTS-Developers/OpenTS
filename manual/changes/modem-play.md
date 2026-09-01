---
title: Remove modem and null-modem play
category: feature
release: 0.1.0
breaking: true
migration:
- Play over a network, the Internet, or in skirmish instead. A modem or null-modem game can no longer be started.
- Delete the `[SerialDefaults]`, `[InitStrings]` and `[PhoneBook]` sections and `[MultiPlayer] PhoneIndex` from `sun.ini`, or leave them to be ignored.
targets:
- type: command
  id: fixed:cancel-modem-operation
  effect: removed
- type: key
  id: ModemName
  effect: removed
- type: key
  id: Port
  effect: removed
- type: key
  id: IRQ
  effect: removed
- type: key
  id: Baud
  effect: removed
- type: key
  id: Compression
  effect: removed
- type: key
  id: ErrorCorrection
  effect: removed
- type: key
  id: DialMethod
  effect: removed
- type: key
  id: InitStringIndex
  effect: removed
- type: key
  id: CallWaitStringIndex
  effect: removed
- type: key
  id: CallWaitString
  effect: removed
- type: key
  id: PhoneIndex
  effect: removed
credit: [ZivDero]
---

Games played over a modem or a null-modem cable are no longer supported. The
serial connection, its dialing and answering screens, the phone book, and the
modem settings editor are all gone. Network, internet, skirmish, and campaign
play are unaffected.

`sun.ini` no longer reads or writes the `[SerialDefaults]`, `[InitStrings]`, and
`[PhoneBook]` sections, nor `[MultiPlayer] PhoneIndex`, and stops rewriting them
when it saves its settings.

The two session types the removed modes used keep their stored values, so saves
are unaffected.
