param([switch]$SkipVigem, [ValidateSet('en','pt-BR')][string]$Language = 'en')
$ErrorActionPreference = 'Stop'
function Text($English, $Portuguese) { if ($Language -eq 'pt-BR') { $Portuguese } else { $English } }
Set-Location $PSScriptRoot
cmake -S . -B build-windows -A x64
if ($LASTEXITCODE -ne 0) { throw (Text 'CMake failed. Install Visual Studio 2022+ with Desktop development with C++ and CMake.' 'CMake falhou. Instale Visual Studio 2022+ com Desktop development with C++ e CMake.') }
cmake --build build-windows --config Release
if ($LASTEXITCODE -ne 0) { throw (Text 'Build failed.' 'Compilação falhou.') }
ctest --test-dir build-windows -C Release --output-on-failure
if ($LASTEXITCODE -ne 0) { throw (Text 'Tests failed.' 'Testes falharam.') }
if (!$SkipVigem) { & (Join-Path $PSScriptRoot 'tools\build-vigem-client.ps1') -Language $Language }
Write-Host (Text 'Ready. Run .\run-windows.ps1' 'Pronto. Execute .\run-windows.ps1')
