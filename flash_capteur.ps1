# Compile et televerse le programme du noeud capteur, puis affiche son moniteur serie.
# Usage : .\flash_capteur.ps1          (port COM9 par defaut)
#         .\flash_capteur.ps1 COM8
param([string]$Port = "COM9")

$cli = "C:\Program Files\Arduino CLI\arduino-cli.exe"
$sketch = Join-Path $PSScriptRoot "capteur"

& $cli compile --fqbn esp8266:esp8266:nodemcuv2 --upload -p $Port $sketch
if ($LASTEXITCODE -ne 0) { Write-Host "Echec du televersement."; exit 1 }

Write-Host "`nMoniteur serie (Ctrl+C pour quitter) :"
& $cli monitor -p $Port --config baudrate=115200
