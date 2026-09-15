---
key: Capturable
summary: Allows a structure to take the enter cursor and to change hands when an engineer walks in.
see_also: ["system:capture"]
when_omitted:
  kind: value
  value: "no"
---

Three paths read it: the engineer's own cursor over a [`Repairable=yes`](/keys/repairable/) structure, the enter cursor every [`Infiltrate=yes`](/keys/infiltrate/) soldier is offered over a non-allied structure, and the capture itself. Because the last of the three is a separate test from the first two, a structure that somehow receives the order without the cursor still refuses to change hands. The engineer is consumed and nothing happens.

Two paths bypass it entirely. A structure that undeploys into a vehicle is [captured through the vehicle branch](/systems/capture/#the-vehicle-branch), which tests nothing but the destination. And the skirmish branch that has an engineer damage a structure instead of taking it does not read the setting either, so `Capturable=no` does not protect a structure from that damage.

A computer house's scan reads it too, as the filter on [a request for capturable structures](/systems/target-selection/#why-a-candidate-is-rejected).
