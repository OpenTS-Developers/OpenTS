---
title: Print an at sign in text that breaks lines at one
category: feature
release: 0.2.0
targets:
- type: format
  id: ini-syntax
  effect: changed
credit:
- ZivDero
---

In tooltips, written briefings and the map selection screens, where an at sign (`@`) starts a new line, `\@` now prints an at sign and `\\` prints a backslash. Text without these escapes prints as before.
