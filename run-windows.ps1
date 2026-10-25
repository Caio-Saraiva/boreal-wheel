param([switch]$Demo, [int]$Port = 4580)
$ErrorActionPreference = 'Stop'
Set-Location $PSScriptRoot
$release = Join-Path $PSScriptRoot 'build-windows\Release'
$exe = Join-Path $release 'boreal-wheel.exe'
if (!(Test-Path $exe)) { throw 'Compile primeiro com .\build-windows.ps1' }
Set-Location $release
Start-Process "http://127.0.0.1:$Port"
$arguments = @('--port', "$Port")
if ($Demo) { $arguments += '--demo' }
& $exe @arguments
