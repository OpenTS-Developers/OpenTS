---
enum_id: PipEnum
slug: pip-color
title: Pip color
summary: Named pip colors accepted for object status markers.
representation: token
bindings:
  key_value_types: [pipenum]
  scripting_parameter_types: []
source_files: [code/pip.hh, code/ccini.cpp]
values:
  - { constant: PIP_EMPTY, value: 0, input: "empty", meaning: "Empty pip slot." }
  - { constant: PIP_GREEN, value: 1, input: "green", meaning: "Green filled pip." }
  - { constant: PIP_YELLOW, value: 2, input: "yellow", meaning: "Yellow filled pip." }
  - { constant: PIP_WHITE, value: 3, input: "white", meaning: "White filled pip." }
  - { constant: PIP_RED, value: 4, input: "red", meaning: "Red filled pip." }
  - { constant: PIP_BLUE, value: 5, input: "blue", meaning: "Blue filled pip." }
---

A **pip** is one of the small markers drawn in a row beneath a selected object, and these six colors are what [`Pip`](/keys/pip/) may name; the match ignores case. The engine's own list runs on past them, with a medic's cross, a veteran mark, an elite mark and three health colors it picks for itself. No rules file can ask for any of them.
