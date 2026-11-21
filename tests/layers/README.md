# Layer references

`check_layers.py` reports the references between engine source files that
point against a proposed layer order. The order is a proposal: the sources
have not moved, CTest does not run the script, and nothing fails when a
reference is added.

[`layers.toml`](layers.toml) assigns each source file to a layer, by directory
for `code/audio/`, `code/sdl/`, and `code/ui/` and by name for every other
file, and lists the layers each layer may use:

| Layer | Holds | May use |
| --- | --- | --- |
| `core` | Files, streams, compression, INI, timers, math, the save stream | nothing |
| `platform` | Window, input, sockets, the crash handler | `core` |
| `gfx` | Surfaces, blitters, palettes, shapes, fonts, voxels, video decoding | `core`, `platform` |
| `audio` | The mixer, decoders, streams, and output device | `core`, `platform` |
| `net` | Transport, packets, timing | `core`, `platform` |
| `sim` | Everything deterministic: objects, types, houses, rules, map cells, logic, triggers | `core` |
| `ui` | RmlUi screens and the inherited gadgets, dialogs, and menus | `core`, `platform`, `gfx`, `audio`, `sim` |
| `view` | The in-game screen: tactical view, display stack, sidebar, radar, sound cues, movies | every layer above except `net` |
| `app` | Startup, the main loop, session setup, the save manager, headless tasks | every layer |

## Running it

Build the engine in Debug, then pass the directory that holds its object
files:

```powershell
python tests/layers/check_layers.py --objects build/code/OpenTS.dir/Debug
```

The script prints the number of file pairs with a reference against the
order, for each pair of layers. `--list` also prints each pair of files:

```text
sim/techno -> view/tactical
```

The script stops with an error when a source file has no layer or
`layers.toml` names a file that does not exist. Add the name of a new source
file, without its extension, to a `files` list.

A Release build drops a few references that Debug keeps, so compare runs of
the same configuration.

## What it measures

The script reads the symbol table of each object file. It counts a reference
when one source file uses a function or variable that exactly one other source
file defines.

It does not see includes, virtual calls, inheritance, or code that lives in a
header, such as inline functions and templates. A layer with no reported
reference can therefore still fail to link without the layers above it.

## Comparing against a saved list

`--baseline <file> --update` writes the current pairs of files to a file.
`--baseline <file>` then exits with a failure when a pair not in the file has
a reference against the order, or when a line in the file no longer applies;
`--stale allow` passes the second case. A second reference between a listed
pair passes.

`python tests/layers/test_check_layers.py` runs the script's own tests. Both
need Python 3.11 or newer.
