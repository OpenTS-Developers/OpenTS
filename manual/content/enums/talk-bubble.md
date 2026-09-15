---
enum_id: TalkType
slug: talk-bubble
title: Talk bubble
summary: Speech-bubble styles that can be hung over a scripted character.
representation: integer
bindings:
  key_value_types: []
  scripting_parameter_types: [talk-bubble]
source_files: [code/talk.hh, code/techno.cpp, code/objtype.cpp]
values:
  - { constant: TALK_NONE, value: 0, input: "0", meaning: "No talk bubble." }
  - { constant: TALK_1, value: 1, input: "1", meaning: "Standard talk bubble." }
  - { constant: TALK_QUESTION, value: 2, input: "2", meaning: "Question-mark bubble." }
  - { constant: TALK_EXCLAMATION, value: 3, input: "3", meaning: "Exclamation-mark bubble." }
---

The figure is a frame number in `TALKBUBL.SHP`, one higher than the frame drawn: figure `1` draws frame `0`. The three bubbles are therefore the first three shapes in that file, so swapping the file changes what each figure shows. A figure above `3` asks for a frame further into the file. Where the file holds no such frame, nothing is drawn at all.

[`TalkBubbleTime`](/keys/talkbubbletime/) covers how long a bubble stays up, what else placing one does, and how a second one displaces the first.
