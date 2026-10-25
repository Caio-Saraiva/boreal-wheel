param([switch]$SkipVigem)
$ErrorActionPreference = 'Stop'
Set-Location $PSScriptRoot
cmake -S . -B build-windows -A x64
if ($LASTEXITCODE -ne 0) { throw 'CMake falhou. Instale Visual Studio 2022+ com Desktop development with C++ e CMake.' }
cmake --build build-windows --config Release
if ($LASTEXITCODE -ne 0) { throw 'Compilação falhou.' }
ctest --test-dir build-windows -C Release --output-on-failure
if ($LASTEXITCODE -ne 0) { throw 'Testes falharam.' }
if (!$SkipVigem) { & (Join-Path $PSScriptRoot 'tools\build-vigem-client.ps1') }
Write-Host 'Pronto. Execute .\run-windows.ps1'
