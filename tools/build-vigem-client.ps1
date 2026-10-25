param([string]$Destination = (Join-Path $PSScriptRoot '..\build-windows\Release'))
$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$commit = 'b66d02d57e32cc8595369c53418b843e958649b4'
$sha256 = 'f1d34de29007e0fb17a15f1656c8b84c500bf33aee4e30d8317df1368e76e36c'
$deps = Join-Path $root '.deps\vigem'
New-Item -ItemType Directory -Force -Path $deps | Out-Null
$archive = Join-Path $deps 'source.zip'
if (!(Test-Path $archive)) {
  Invoke-WebRequest "https://codeload.github.com/nefarius/ViGEmClient/zip/$commit" -OutFile $archive
}
if ((Get-FileHash $archive -Algorithm SHA256).Hash.ToLowerInvariant() -ne $sha256) {
  throw 'SHA256 do código oficial não corresponde. Remova .deps/vigem/source.zip e confira a origem.'
}
$source = Join-Path $deps "ViGEmClient-$commit"
if (!(Test-Path $source)) { Expand-Archive $archive -DestinationPath $deps }
$build = Join-Path $deps 'build'
cmake -S (Join-Path $PSScriptRoot 'vigem') -B $build -A x64 "-DVIGEM_SOURCE=$source"
if ($LASTEXITCODE -ne 0) { throw 'Falha ao configurar ViGEmClient.' }
cmake --build $build --config Release --target ViGEmClientShared
if ($LASTEXITCODE -ne 0) { throw 'Falha ao compilar ViGEmClient.' }
New-Item -ItemType Directory -Force -Path $Destination | Out-Null
Copy-Item (Join-Path $build 'upstream\Release\ViGEmClient.dll') $Destination -Force
Copy-Item (Join-Path $source 'LICENSE') (Join-Path $Destination 'LICENSE-ViGEmClient.txt') -Force
Write-Host 'ViGEmClient.dll x64 compilada. O driver ViGEmBus é instalado separadamente.'
