# Project direction

The [README](../README.md) tracks current development priorities. This page
explains the architectural direction behind them.

## Toward an entity-component architecture

The inherited object model puts state and behavior in one deep class hierarchy,
leaving much of the original game hard-coded. OpenTS is moving gradually toward
an entity-component architecture to reduce hard-coding and make the engine
easier to extend.

There will be no full rewrite. Changes must keep the engine playable throughout
the migration. Prefer composition to a deeper inheritance tree, keep new state
separable from behavior, and avoid adding tight couplings to the existing class
hierarchy.

Portability follows the same incremental approach. Preparatory changes do not
make a compiler or platform supported; [Building OpenTS](BUILDING.md) lists the
current target.
