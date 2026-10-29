---
name: Debug launch
description: Start the x64 Debug GameD.exe against the game data in C:\BoomerGames\siliconsun. One PowerShell command. Does not build.
---

# Debug launch

Start the x64 Debug client against `C:\BoomerGames\siliconsun`. Run this from
the repository root when the user asks to launch, play, or debug that data
directory. Do not build first, and do not copy the executable into the data
directory.

```powershell
Start-Process -FilePath "$PWD\build\x64\bin\Debug\GameD.exe" -ArgumentList "-DATADIR=C:\BoomerGames\siliconsun"
```

`Start-Process` returns once the process is created. The game reads
`C:\BoomerGames\siliconsun` and does not write there. Settings, saves, the
debug log, and crash reports go beside `GameD.exe`. `Language.dll` and `ui/`
in `build\x64\bin\Debug` are the ones that run.

The executable is `build\x64\bin\Debug\GameD.exe`. If that file is missing,
stop and say so. `build\x64` is already configured for x64. Building the
missing Debug client is:

```powershell
cmake --build build/x64 --config Debug
```
