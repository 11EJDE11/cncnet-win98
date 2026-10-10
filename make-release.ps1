# Packages build\cncnet-win98-<version>.zip: CNCNET98.BAT and the Win98 folder (to be extracted
# into the game folder), plus README and LICENSE.
#   .\make-release.ps1 -Version 1.0
param([Parameter(Mandatory = $true)][string]$Version)

$ErrorActionPreference = 'Stop'
$root = $PSScriptRoot
$stage = "$root\build\release\cncnet-win98-$Version"
$zip = "$root\build\cncnet-win98-$Version.zip"
Remove-Item -Recurse -Force $stage, $zip -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force "$stage\Win98" | Out-Null
Copy-Item "$root\CNCNET98.BAT", "$root\README.md", "$root\LICENSE" $stage
Copy-Item "$root\Win98\*" "$stage\Win98" -Exclude DL, WORK, SYRINGE.ORG

# command.com needs CRLF line endings; convert byte-wise.
Get-ChildItem $stage -Recurse -Include *.bat | ForEach-Object {
    $b = [IO.File]::ReadAllBytes($_.FullName)
    $out = New-Object System.Collections.Generic.List[byte]
    for ($i = 0; $i -lt $b.Length; $i++) {
        if ($b[$i] -eq 10 -and ($i -eq 0 -or $b[$i - 1] -ne 13)) { $out.Add(13) }
        $out.Add($b[$i])
    }
    [IO.File]::WriteAllBytes($_.FullName, $out.ToArray())
}
# The zip holds the files at its top level, so extracting it into the game folder is enough.
Compress-Archive -Path "$stage\*" -DestinationPath $zip
Write-Host "wrote $zip"
