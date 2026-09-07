---
title: Read and show text as UTF-8
category: feature
release: 0.2.0
targets:
- type: format
  id: ini-syntax
  effect: changed
credit:
- ZivDero
---

Game text, INI files, typed input, player names and chat are UTF-8. The shipped fonts draw the Western European letters and symbols they carry and show `?` for anything else, so `é` and `œ` render but Cyrillic and Greek do not. The 12-point metal font, which draws the sidebar and the credits roll, drew the wrong glyph for an accented letter; the title-screen copyright and the first line of the command-line usage text showed `Â©`.

An INI file that is not valid UTF-8 is read as Windows-1252, so existing maps and mods keep their accented names and a file whose digest was written in that code page still verifies. Saving writes UTF-8 without a byte order mark.

Player names now hold 64 bytes and chat lines 224, where an accented Latin character takes two bytes. The UTF-8 code page needs Windows 10 version 1903 or newer; older Windows keeps its own code page, where the game's text still renders.
