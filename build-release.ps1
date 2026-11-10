param([string]$BuildDir = "build")
$ErrorActionPreference = "Stop"
$vs2026CMake = "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
$vs2022CMake = "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
if (Test-Path $vs2026CMake) { $cmake = $vs2026CMake; $generator = "Visual Studio 18 2026" }
elseif (Test-Path $vs2022CMake) { $cmake = $vs2022CMake; $generator = "Visual Studio 17 2022" }
else { $cmake = "cmake"; $generator = "Visual Studio 18 2026" }
$ctest = Join-Path (Split-Path $cmake) "ctest.exe"
& $cmake -S . -B $BuildDir -G $generator -A x64
if($LASTEXITCODE -ne 0){throw "Configure failed"}
& $cmake --build $BuildDir --config Release --target sewerline -- /m:1
if($LASTEXITCODE -ne 0){throw "Release build failed"}
& $cmake --build $BuildDir --config Debug --target sewerline_tests -- /m:1
if($LASTEXITCODE -ne 0){throw "Test build failed"}
& $ctest --test-dir $BuildDir -C Debug --output-on-failure
if($LASTEXITCODE -ne 0){throw "Tests failed"}
& "$PSScriptRoot/tools/package-release.ps1" -BuildDir $BuildDir
Write-Host "Self-contained playable EXE and credits are in release."
