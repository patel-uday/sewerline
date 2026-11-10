param([string]$BuildDir = "build")
$ErrorActionPreference = "Stop"
$projectRoot = Split-Path $PSScriptRoot
$release = Join-Path $projectRoot "release"
New-Item -ItemType Directory -Force -Path $release | Out-Null
Copy-Item -LiteralPath (Join-Path $BuildDir "Release/sewerline.exe") -Destination $release -Force
Copy-Item -LiteralPath (Join-Path $projectRoot "LICENSE.txt") -Destination $release -Force
Copy-Item -LiteralPath (Join-Path $projectRoot "assets/audio/LICENSES.md") -Destination (Join-Path $release "AUDIO-LICENSES.md") -Force
Copy-Item -LiteralPath (Join-Path $BuildDir "_deps/sdl-src/LICENSE.txt") -Destination (Join-Path $release "SDL-LICENSE.txt") -Force
Get-Item (Join-Path $release "sewerline.exe") | Select-Object Name,Length
Write-Host "The EXE contains the engine library, C++ runtime, textures, sprites and sounds."
