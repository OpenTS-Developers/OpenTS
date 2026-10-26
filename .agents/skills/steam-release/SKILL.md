---
name: Steam release deploy
description: Rebuild the Win32 Release from the current checkout and deploy it to the Steam Tiberian Sun install, replacing SUN.EXE, Language.dll, Game.pdb, and ui/.
---

# Steam release deploy

Use this skill when the user asks to rebuild a release and copy it to Steam.

## Locations

| Item | Path |
| --- | --- |
| Build output | `build/bin/Release/` |
| Steam install | `C:\Program Files (x86)\Steam\steamapps\common\Command & Conquer Tiberian Sun` |
| Original Steam files | `_steam_backup/` inside the Steam install |

Steam launches `SUN.EXE`, so OpenTS's `Game.exe` is deployed under that name.
The deployment also replaces `Language.dll`, `Game.pdb`, and the whole `ui/`
directory. Steam's own `Game.exe` stays in place.

## Preconditions

- Close the game. The deploy script refuses to run while `SUN` or `Game` is
  running.
- Require `_steam_backup\SUN.EXE`, `_steam_backup\Language.dll`, and
  `_steam_backup\SUN.INI`. Stop and ask the user to restore them when one is
  missing; otherwise the deployment overwrites the only copy of the original
  files.
- Confirm the build directory targets Win32:

  ```powershell
  Select-String '^CMAKE_GENERATOR_PLATFORM:INTERNAL=Win32$' build\CMakeCache.txt
  ```

  If there is no match, stop and configure a Win32 build as
  `docs/BUILDING.md` describes. Do not configure x64 in `build/`.

## Build

```powershell
cmake --build build --config Release
```

A successful build links `build\bin\Release\Game.exe`. Check the log for
`: error `; inherited warnings are normal. When the link fails or an error
appears, report it and stop without deploying.

## Deploy

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .agents\skills\steam-release\scripts\deploy.ps1
```

The script copies the executable to `SUN.EXE`, copies `Language.dll` and
`Game.pdb`, deletes the old `ui/` before copying the new one, and fails when a
copied file differs from the build. Deleting `ui/` first matters: copying a
directory over an existing one nests it as `ui/ui`.

Pass `-SteamDir` when the install lives elsewhere. A missing directory makes
the script fail before it copies anything.

## Verify and report

The script prints the commit, worktree state, executable size and version, hash
matches, UI file count, and backup contents. Report those values. When the
script throws, report the failing check and leave the install as it is.

## Tests

The script does not run CTest. Run the Release tests when the user asks for a
checked build or the change affects simulation behavior:

```powershell
ctest --test-dir build -C Release --output-on-failure
```

## Compatibility

Each deployment is a development snapshot. Saves from an older build may fail
to load, and a Win32 build cannot share saves or network games with x64. Mention
this when the user asks about existing saves or multiplayer.
