param([switch]$Demo, [int]$Port = 4580, [ValidateSet('en','pt-BR')][string]$Language = 'en')
$ErrorActionPreference = 'Stop'
Set-Location $PSScriptRoot
$release = Join-Path $PSScriptRoot 'build-windows\Release'
$exe = Join-Path $release 'boreal-wheel.exe'
if (!(Test-Path $exe)) {
  if ($Language -eq 'pt-BR') { throw 'Compile primeiro com .\build-windows.ps1' }
  throw 'Build first with .\build-windows.ps1'
}
Set-Location $release
Start-Process "http://127.0.0.1:$Port"
$arguments = @('--port', "$Port", '--lang', $Language)
if ($Demo) { $arguments += '--demo' }
& $exe @arguments
