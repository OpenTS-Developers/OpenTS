<#
.SYNOPSIS
Copy an OpenTS x64 Release build into the Steam Tiberian Sun install.

.DESCRIPTION
Steam launches SUN.EXE, so Game.exe is copied under that name. Language.dll,
Game.pdb, and ui/ are replaced as well. The original Steam files stay in
_steam_backup.

Run this after `cmake --build build\x64 --config Release`.
#>
[CmdletBinding()]
param(
    [string]$SteamDir = 'C:\Program Files (x86)\Steam\steamapps\common\Command & Conquer Tiberian Sun',
    [string]$BuildDir = '',
    [string]$RepoRoot = ''
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

if (-not $RepoRoot) {
    $RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..\..')).Path
}
if (-not $BuildDir) {
    $BuildDir = Join-Path $RepoRoot 'build\x64\bin\Release'
}

foreach ($name in 'Game.exe', 'Language.dll', 'Game.pdb', 'ui') {
    $path = Join-Path $BuildDir $name
    if (-not (Test-Path -LiteralPath $path)) {
        throw "Missing build output: $path. Build the x64 Release first."
    }
}

foreach ($name in 'Game.exe', 'Language.dll') {
    $path = Join-Path $BuildDir $name
    $arch = Get-PEArchitecture $path
    if ($arch -ne 'x64') {
        throw "$path is $arch. Build the x64 Release in $BuildDir, not the Win32 build."
    }
}
if (-not (Test-Path -LiteralPath $SteamDir)) {
    throw "Steam install not found: $SteamDir"
}

$backup = Join-Path $SteamDir '_steam_backup'
foreach ($name in 'SUN.EXE', 'Language.dll', 'SUN.INI') {
    if (-not (Test-Path -LiteralPath (Join-Path $backup $name))) {
        throw "Missing backup: $(Join-Path $backup $name). Back up the Steam files before deploying."
    }
}

$running = @(Get-Process SUN, Game -ErrorAction SilentlyContinue)
if ($running.Count -gt 0) {
    throw "Close the game first: $($running.Name -join ', ')"
}

foreach ($file in @(
    @{ Source = 'Game.exe'; Destination = 'SUN.EXE' },
    @{ Source = 'Language.dll'; Destination = 'Language.dll' },
    @{ Source = 'Game.pdb'; Destination = 'Game.pdb' }
)) {
    $source = Join-Path $BuildDir $file.Source
    $destination = Join-Path $SteamDir $file.Destination
    Copy-Item -LiteralPath $source -Destination $destination -Force
    if ((Get-FileHash -LiteralPath $source).Hash -ne (Get-FileHash -LiteralPath $destination).Hash) {
        throw "$($file.Destination) does not match $source after copying"
    }
}

$sourceUi = Join-Path $BuildDir 'ui'
$targetUi = Join-Path $SteamDir 'ui'
if (Test-Path -LiteralPath $targetUi) {
    Remove-Item -LiteralPath $targetUi -Recurse -Force
}
Copy-Item -LiteralPath $sourceUi -Destination $targetUi -Recurse -Force

$sourceUiCount = @(Get-ChildItem -LiteralPath $sourceUi -Recurse -File).Count
$targetUiCount = @(Get-ChildItem -LiteralPath $targetUi -Recurse -File).Count
if ($sourceUiCount -ne $targetUiCount) {
    throw "ui/ has $targetUiCount files; the build has $sourceUiCount"
}
if (Test-Path -LiteralPath (Join-Path $targetUi 'ui')) {
    throw "ui/ was copied into itself: $targetUi\ui exists"
}

$exe = Get-Item -LiteralPath (Join-Path $SteamDir 'SUN.EXE')
$dirty = @(git -C $RepoRoot status --short)
Write-Output ("repo HEAD     : {0}" -f (git -C $RepoRoot log -1 --format='%h %s'))
Write-Output ("worktree      : {0}" -f $(if ($dirty.Count -eq 0) { 'clean' } else { "$($dirty.Count) changed path(s)" }))
Write-Output ("SUN.EXE       : {0:N0} bytes  {1}" -f $exe.Length, $exe.LastWriteTime)
Write-Output ("arch          : {0}" -f (Get-PEArchitecture $exe.FullName))
Write-Output ("version       : {0} {1}" -f $exe.VersionInfo.FileDescription, $exe.VersionInfo.FileVersion)
Write-Output ("exe hash match: {0}" -f ((Get-FileHash -LiteralPath (Join-Path $BuildDir 'Game.exe')).Hash -eq (Get-FileHash -LiteralPath $exe.FullName).Hash))
Write-Output ("lang hash     : {0}" -f ((Get-FileHash -LiteralPath (Join-Path $BuildDir 'Language.dll')).Hash -eq (Get-FileHash -LiteralPath (Join-Path $SteamDir 'Language.dll')).Hash))
Write-Output ("pdb hash      : {0}" -f ((Get-FileHash -LiteralPath (Join-Path $BuildDir 'Game.pdb')).Hash -eq (Get-FileHash -LiteralPath (Join-Path $SteamDir 'Game.pdb')).Hash))
Write-Output ("ui files      : {0}  nested ui: {1}" -f $targetUiCount, (Test-Path -LiteralPath (Join-Path $targetUi 'ui')))
Write-Output ("backup        : {0}" -f ((Get-ChildItem -LiteralPath $backup).Name -join ', '))
