# Rebuilds this project's DLLs into Win98\ with Visual C++ 6.0 (VC98).
param([string]$Out = "$PSScriptRoot\Win98")

$ErrorActionPreference = 'Stop'
$vc = "C:\Program Files (x86)\Microsoft Visual Studio\VC98"
if (-not (Test-Path "$vc\Bin\CL.EXE")) { throw "Visual C++ 6.0 not found at $vc" }
$env:INCLUDE = "$vc\Include"
$env:LIB     = "$vc\Lib"
$env:PATH    = "$vc\Bin;C:\Program Files (x86)\Microsoft Visual Studio\Common\MSDev98\Bin;$env:PATH"
$obj = "$PSScriptRoot\build"
New-Item -ItemType Directory -Force $Out, $obj | Out-Null

function Build-Dll([string]$name, [string]$dll, [string[]]$libs) {
    $o = "$obj\$name.obj"
    & "$vc\Bin\CL.EXE" /nologo /c /O2 /MT /DWIN32 /D_WINDOWS /DNDEBUG "$PSScriptRoot\src\$name.c" "/Fo$o"
    if ($LASTEXITCODE) { throw "compile failed: $name" }
    & "$vc\Bin\LINK.EXE" /nologo /DLL "/DEF:$PSScriptRoot\src\$name.def" "/OUT:$Out\$dll" "/IMPLIB:$obj\$name.lib" $o @libs
    if ($LASTEXITCODE) { throw "link failed: $name" }
    Write-Host "built $Out\$dll"
}

$base = 'kernel32.lib', 'user32.lib', 'advapi32.lib'
Build-Dll olet3          OLET3.DLL     ($base + 'ole32.lib')
Build-Dll ntdta          NTDTA.DLL     $base
Build-Dll xaudio2_6_noop XAudio2_6.dll ($base + 'ole32.lib')
Get-ChildItem $Out -Include *.exp, *.lib -Recurse | Remove-Item
