<#
.SYNOPSIS
Copy Tiberian Sun game data from retail discs or an existing install into one folder.

.DESCRIPTION
Reads the game files from one or more folders, such as mounted discs, mounted
ISOs, an extracted disc, or an installed copy of the game, and copies the files
OpenTS uses into the target folder. Movies and music are copied unless
-NoMovies or -NoMusic is given. Expansion files are copied unless -NoFirestorm
is given.

A retail disc keeps the base game archives inside INSTALL\TIBSUN.MIX. That file
is copied as TIBSUN.MIX, the layout the Steam and First Decade installs use, and
the game reads the archives it holds.

With -BuildDir, the x64 Release build is copied beside the data so the folder
runs on its own; the default BuildDir is build\x64\bin\Release when it exists,
and -NoBuild skips the copy. Use -WhatIf to print the plan without writing
anything.
#>
[CmdletBinding(SupportsShouldProcess = $true)]
param(
    [string[]]$Source = @(),
    [string]$TargetDir = (Join-Path $HOME 'Games\OpenTS'),
    [string]$BuildDir = '',
    [switch]$NoMovies,
    [switch]$NoMusic,
    [switch]$NoFirestorm,
    [switch]$NoBuild
)

$ErrorActionPreference = 'Stop'

function Get-PEArchitecture {
    param([string]$Path)

    $stream = [IO.File]::OpenRead($Path)
    try {
        $reader = New-Object IO.BinaryReader($stream)
        $stream.Position = 0x3C
        $peOffset = $reader.ReadInt32()
        $stream.Position = $peOffset + 4
        $machine = $reader.ReadUInt16()
    } finally {
        $stream.Dispose()
    }

    switch ($machine) {
        0x8664 { 'x64' }
        0x014C { 'x86' }
        0xAA64 { 'arm64' }
        default { 'unknown' }
    }
}

function Format-Megabytes {
    param([long]$Bytes)

    return ($Bytes / 1MB).ToString('N1', [Globalization.CultureInfo]::InvariantCulture)
}

$groups = [ordered]@{
    Core      = @(
        'TIBSUN.MIX', 'CACHE.MIX', 'CONQUER.MIX', 'SOUNDS.MIX', 'LOCAL.MIX',
        'MULTI.MIX', 'PATCH.MIX', 'PCACHE.MIX',
        'MAPS??.MIX', 'SIDEC??.MIX', 'SIDENC??.MIX', 'SIDECD??.MIX', 'SPEECH??.MIX',
        'TEMPERAT.MIX', 'TEM.MIX', 'SNOW.MIX', 'SNO.MIX', 'ISOTEMP.MIX', 'ISOSNOW.MIX',
        '*.MAP'
    )
    Movies    = @('MOVIES??.MIX', '*.VQA')
    Music     = @('SCORES.MIX', 'SCORES??.MIX')
    Firestorm = @(
        'EXPAND??.MIX', 'ECACHE??.MIX',
        'E??SC??.MIX', 'E??SCD??.MIX', 'E??SNC??.MIX', 'E??VOX??.MIX',
        'SOUNDS01.MIX', 'GMENU.MIX', 'WDT.MIX', 'WDTVOX.MIX'
    )
}

function Get-FileGroup {
    param([string]$Name)

    # A build output's Game.map must not pass for a map.
    if ($Name -eq 'GAME.MAP') {
        return $null
    }
    foreach ($group in $groups.Keys) {
        foreach ($pattern in $groups[$group]) {
            if ($Name -like $pattern) {
                return $group
            }
        }
    }
    return $null
}

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..\..')).Path
if ($TargetDir -match '^~([\\/]|$)') {
    $rest = $TargetDir.Substring(1).TrimStart([char[]]@('\', '/'))
    if ($rest) {
        $TargetDir = Join-Path $HOME $rest
    } else {
        $TargetDir = $HOME
    }
}
if (-not $NoBuild -and -not $BuildDir) {
    $candidate = Join-Path $repoRoot 'build\x64\bin\Release'
    if (Test-Path -LiteralPath (Join-Path $candidate 'Game.exe')) {
        $BuildDir = $candidate
    }
}

$mounted = New-Object System.Collections.ArrayList
$roots = New-Object System.Collections.ArrayList

try {
    if ($Source.Count -eq 0) {
        foreach ($drive in [IO.DriveInfo]::GetDrives()) {
            if ($drive.DriveType -eq [IO.DriveType]::CDRom -and $drive.IsReady) {
                $roots.Add($drive.RootDirectory.FullName) | Out-Null
            }
        }
        if ($roots.Count -eq 0) {
            throw 'No disc drive holds a readable disc. Insert a Tiberian Sun disc, or pass -Source with a folder or an .iso file.'
        }
    } else {
        foreach ($path in $Source) {
            if ($path -match '\.iso$') {
                try {
                    $image = Mount-DiskImage -ImagePath (Resolve-Path -LiteralPath $path).Path -PassThru
                } catch {
                    throw "Cannot mount $path. Mount the ISO in Explorer and pass its drive letter, or run this script as administrator. $($_.Exception.Message)"
                }
                $mounted.Add($image) | Out-Null
                $letter = (Get-Volume -DiskImage $image).DriveLetter
                if (-not $letter) {
                    throw "Mounted $path but found no drive letter."
                }
                $roots.Add($letter + ':\') | Out-Null
            } else {
                $resolved = (Resolve-Path -LiteralPath $path -ErrorAction Stop).Path
                if (-not (Test-Path -LiteralPath $resolved -PathType Container)) {
                    throw "Source is not a folder or an .iso file: $path"
                }
                $roots.Add($resolved) | Out-Null
            }
        }
    }

    $targetPath = [IO.Path]::GetFullPath($TargetDir)
    $selected = @{}
    $replaced = New-Object System.Collections.ArrayList

    foreach ($root in $roots) {
        $rootPath = (Resolve-Path -LiteralPath $root).Path
        Get-ChildItem -LiteralPath $rootPath -Recurse -File -ErrorAction SilentlyContinue | ForEach-Object {
            $name = $_.Name.ToUpperInvariant()
            $group = Get-FileGroup $name
            if (-not $group) {
                return
            }
            if ($NoMovies -and $group -eq 'Movies') {
                return
            }
            if ($NoMusic -and $group -eq 'Music') {
                return
            }
            if ($NoFirestorm -and $group -eq 'Firestorm') {
                return
            }
            if ($_.FullName.StartsWith($targetPath, [StringComparison]::OrdinalIgnoreCase)) {
                return
            }
            if ($selected.ContainsKey($name) -and $selected[$name].Length -ne $_.Length) {
                $replaced.Add(("{0} from {1}" -f $name, $rootPath)) | Out-Null
            }
            $selected[$name] = [pscustomobject]@{
                Name   = $name
                Group  = $group
                Length = $_.Length
                Path   = $_.FullName
            }
        }
    }

    if ($selected.Count -eq 0) {
        throw "No game data found in $($roots -join ', '). Pass the folder that holds the installed game, or a disc that holds INSTALL\TIBSUN.MIX."
    }

    if (-not $selected.ContainsKey('TIBSUN.MIX')) {
        $missing = @('CACHE.MIX', 'CONQUER.MIX', 'SOUNDS.MIX') | Where-Object { -not $selected.ContainsKey($_) }
        if ($missing.Count -gt 0) {
            throw "The source holds neither TIBSUN.MIX nor all of CACHE.MIX, CONQUER.MIX, and SOUNDS.MIX; it is missing $($missing -join ', '). Pass the folder that holds the installed game, or run the disc's setup and pass the folder it installs."
        }
    } elseif ($selected['TIBSUN.MIX'].Length -lt 10MB) {
        Write-Warning ("TIBSUN.MIX is {0} MB. The retail and Steam copies are about 75 MB, so check that the source holds game data and not a mod." -f (Format-Megabytes $selected['TIBSUN.MIX'].Length))
    }

    # TIBSUN.MIX can hold SOUNDS01.MIX, as the Steam and First Decade installs do.
    if (-not $selected.ContainsKey('TIBSUN.MIX') -and $selected.ContainsKey('EXPAND01.MIX') -and -not $selected.ContainsKey('SOUNDS01.MIX')) {
        throw 'EXPAND01.MIX is present but SOUNDS01.MIX is not, and the game stops at startup when it finds Firestorm without SOUNDS01.MIX. Copy SOUNDS01.MIX from the Firestorm disc, or pass -NoFirestorm.'
    }
    $firestormCount = @($selected.Values | Where-Object { $_.Group -eq 'Firestorm' }).Count
    if ($firestormCount -gt 0 -and -not $selected.ContainsKey('EXPAND01.MIX')) {
        Write-Warning 'Firestorm files are present without EXPAND01.MIX. The Firestorm campaign data is probably still inside the disc installer; pass the folder the Firestorm setup installs, or -NoFirestorm to leave the expansion out.'
    }

    $counts = @{}
    $sizes = @{}
    foreach ($group in $groups.Keys) {
        $counts[$group] = 0
        $sizes[$group] = 0
    }

    if (-not (Test-Path -LiteralPath $TargetDir)) {
        if ($PSCmdlet.ShouldProcess($TargetDir, 'Create the target folder')) {
            New-Item -ItemType Directory -Path $TargetDir -Force | Out-Null
        }
    }

    $kept = 0
    foreach ($file in ($selected.Values | Sort-Object Group, Name)) {
        $destination = Join-Path $TargetDir $file.Name
        $counts[$file.Group]++
        $sizes[$file.Group] += $file.Length
        if (Test-Path -LiteralPath $destination) {
            $existing = Get-Item -LiteralPath $destination
            if ($existing.Length -eq $file.Length) {
                $kept++
                continue
            }
        }
        if ($PSCmdlet.ShouldProcess($destination, "Copy $($file.Path)")) {
            Copy-Item -LiteralPath $file.Path -Destination $destination -Force
            $copy = Get-Item -LiteralPath $destination
            if ($copy.Length -ne $file.Length) {
                throw "$($file.Name) holds $($copy.Length) bytes after copying from $($file.Path), expected $($file.Length)."
            }
        }
    }

    $buildCopied = $false
    if ($BuildDir -and -not $NoBuild) {
        $build = (Resolve-Path -LiteralPath $BuildDir -ErrorAction Stop).Path
        foreach ($name in 'Game.exe', 'Language.dll') {
            $path = Join-Path $build $name
            if (-not (Test-Path -LiteralPath $path)) {
                throw "Missing build output: $path. Build the x64 Release first, or pass -NoBuild."
            }
            $arch = Get-PEArchitecture $path
            if ($arch -ne 'x64') {
                throw "$path is $arch. Build the x64 Release in build\x64, not the Win32 build, or pass -NoBuild."
            }
        }
        if (-not (Test-Path -LiteralPath (Join-Path $build 'ui'))) {
            throw "Missing build output: $(Join-Path $build 'ui')."
        }

        foreach ($name in 'Game.exe', 'Language.dll', 'Game.pdb') {
            $source = Join-Path $build $name
            if (-not (Test-Path -LiteralPath $source)) {
                continue
            }
            $destination = Join-Path $TargetDir $name
            if ($PSCmdlet.ShouldProcess($destination, "Copy $source")) {
                Copy-Item -LiteralPath $source -Destination $destination -Force
                if ((Get-Item -LiteralPath $destination).Length -ne (Get-Item -LiteralPath $source).Length) {
                    throw "$name does not match $source after copying"
                }
            }
            $buildCopied = $true
        }

        $uiSource = Join-Path $build 'ui'
        $uiTarget = Join-Path $TargetDir 'ui'
        if ($PSCmdlet.ShouldProcess($uiTarget, 'Replace the ui folder')) {
            if (Test-Path -LiteralPath $uiTarget) {
                Remove-Item -LiteralPath $uiTarget -Recurse -Force
            }
            Copy-Item -LiteralPath $uiSource -Destination $uiTarget -Recurse -Force
            $sourceCount = @(Get-ChildItem -LiteralPath $uiSource -Recurse -File).Count
            $targetCount = @(Get-ChildItem -LiteralPath $uiTarget -Recurse -File).Count
            if ($sourceCount -ne $targetCount) {
                throw "ui/ holds $targetCount files; the build has $sourceCount"
            }
            if (Test-Path -LiteralPath (Join-Path $uiTarget 'ui')) {
                throw "ui/ was copied into itself: $uiTarget\ui exists"
            }
        }
    }

    Write-Output ("target        : {0}" -f $targetPath)
    foreach ($group in $groups.Keys) {
        if ($counts[$group] -gt 0 -or $sizes[$group] -gt 0) {
            Write-Output ("{0,-13} : {1} file(s), {2} MB" -f $group, $counts[$group], (Format-Megabytes $sizes[$group]))
        }
    }
    foreach ($group in 'Movies', 'Music', 'Firestorm') {
        $skipped = ($group -eq 'Movies' -and $NoMovies) -or ($group -eq 'Music' -and $NoMusic) -or ($group -eq 'Firestorm' -and $NoFirestorm)
        if ($skipped) {
            Write-Output ("{0,-13} : skipped" -f $group)
        } elseif ($counts[$group] -eq 0) {
            Write-Output ("{0,-13} : not found in the source" -f $group)
        }
    }
    if ($kept -gt 0) {
        Write-Output ("kept existing : {0} file(s) already matched" -f $kept)
    }
    if ($replaced.Count -gt 0) {
        Write-Output ("replaced      : {0}" -f ($replaced -join '; '))
    }
    if ($buildCopied) {
        Write-Output ("build         : Game.exe, Language.dll, ui/ from {0}" -f $BuildDir)
        Write-Output ("run           : {0}" -f (Join-Path $TargetDir 'Game.exe'))
    } else {
        Write-Output 'build         : not copied; run the x64 build with -DATADIR pointing at the target'
        Write-Output ("run           : {0} -DATADIR=`"{1}`"" -f (Join-Path $repoRoot 'build\x64\bin\Release\Game.exe'), $TargetDir)
    }
} finally {
    foreach ($image in $mounted) {
        Dismount-DiskImage -ImagePath $image.ImagePath | Out-Null
    }
}
