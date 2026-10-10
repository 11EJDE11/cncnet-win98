@echo off
rem Sets up a CnCNet XNA client (Red Alert 2 / Yuri's Revenge, Dawn of the Tiberium Age, ...)
rem to run on Windows 98 with .NET Framework 4.8 for Windows 98.
rem
rem Put this file and the Win98 folder in the game folder (the one with Resources\clientxna.exe)
rem and run it. Run it again after every CnCNet update: an update can replace files this sets up.
rem
rem It downloads Microsoft's redistributables (checked against known SHA-1 hashes), takes the
rem DLLs the XNA client needs out of them, makes the copies that need it use the .NET for
rem Windows 98 replacements, and puts them beside the client together with this project's own
rem DLLs. Downloads are kept in Win98\DL, so later runs work offline.

set K=C:\DOTNET48
set D=Win98\DL
set W=Win98\WORK
rem URLs and SHA-1 hashes are written out below, not kept in variables: command.com's
rem environment is small and fills up.

echo CnCNet client setup for Windows 98
echo.
if not exist Resources\clientxna.exe goto notgame
if not exist %K%\DN98INST.EXE goto nodotnet
if not exist %K%\7ZA.EXE goto nodotnet
rem DNKERNEL.DLL replaced CORKEL32.DLL in .NET for Windows 98; the DLLs set up here need it.
if not exist %windir%\SYSTEM\DNKERNEL.DLL goto olddotnet
if not exist Win98\OLET3.DLL goto nokit
if not exist %D%\NUL mkdir %D%
if not exist %W%\NUL mkdir %W%

echo Downloading Microsoft's redistributables (skipped if already downloaded) ...
%K%\DN98INST.EXE DOWNLOAD http://download.microsoft.com/download/5/3/A/53A804C8-EC78-43CD-A0F0-2FB4D45603D3/xnafx40_redist.msi %D%\XNAFX40.MSI f35647f4a967db2266176cd5e6b55195882271b9
%K%\DN98INST.EXE DOWNLOAD http://download.microsoft.com/download/1/6/5/165255E7-1014-4D0A-B094-B6A430A6BFFC/vcredist_x86.exe %D%\VCREDIST.EXE 2222fc008e469fec77d0d291877f357c6e1eb16d
%K%\DN98INST.EXE DOWNLOAD http://download.microsoft.com/download/f/f/1/ff178bb1-da91-48ed-89e5-478a99387d4f/wic_x86_enu.exe %D%\WIC.EXE 3e43cf6c7ad781ab2783c4339ab15be4e6143c94
if not exist %D%\XNAFX40.MSI goto dlfail
if not exist %D%\VCREDIST.EXE goto dlfail
if not exist %D%\WIC.EXE goto dlfail

echo Extracting ...
rem XNA Framework 4.0 Refresh: XnaNative, the D3DX9 and X3DAudio it carries, and the managed
rem XNA assemblies (Microsoft_Xna_Framework*_dll_GAC).
%K%\DN98INST.EXE CARVEMSP %D%\XNAFX40.MSI %W%\XNA.CAB > nul
%K%\7ZA.EXE e -y -o%W% %W%\XNA.CAB XnaNative_dll_File MAR09_DXRedist_D3DX9_File DXRedist_X3DAudio_File Microsoft_Xna_Framework*_dll_GAC > nul
%K%\7ZA.EXE e -y -o%W% %W%\MAR09_DXRedist_D3DX9_File d3dx9_41.dll > nul
%K%\7ZA.EXE e -y -o%W% %W%\DXRedist_X3DAudio_File X3DAudio1_7.dll > nul
copy %W%\XnaNative_dll_File %W%\XnaNative.dll > nul
rem Visual C++ 2010 SP1 runtime, version 10.0.40219.325 (from the update inside vcredist).
rem -r: the .msp is stored under a ".\.\" path inside the package.
%K%\7ZA.EXE e -y -r -o%W% %D%\VCREDIST.EXE msp_kb2565063.msp > nul
%K%\DN98INST.EXE CARVEMSP %W%\msp_kb2565063.msp %W%\VC.CAB > nul
%K%\7ZA.EXE e -y -o%W% %W%\VC.CAB F_CENTRAL_msvcr100_x86 F_CENTRAL_msvcp100_x86 > nul
copy %W%\F_CENTRAL_msvcr100_x86 %W%\MSVCR100.dll > nul
copy %W%\F_CENTRAL_msvcp100_x86 %W%\MSVCP100.dll > nul
rem Windows Imaging Component for Windows XP (XNA decodes textures with it).
%K%\7ZA.EXE e -y -o%W% %D%\WIC.EXE > nul

if not exist %W%\XnaNative.dll goto extractfail
if not exist %W%\d3dx9_41.dll goto extractfail
if not exist %W%\windowscodecs.dll goto extractfail
if not exist %W%\MSVCR100.dll goto extractfail
if not exist %W%\MSVCP100.dll goto extractfail

echo Pointing the runtime and WIC DLLs at the Windows 98 replacements ...
%K%\DN98INST.EXE RENAMEIMPORTS %W%\MSVCR100.dll KERNEL32.dll=DNKERNEL.DLL
%K%\DN98INST.EXE RENAMEIMPORTS %W%\MSVCP100.dll KERNEL32.dll=DNKERNEL.DLL
%K%\DN98INST.EXE RENAMEIMPORTS %W%\windowscodecs.dll ole32.dll=OLET3.DLL ntdll.dll=NTDTA.DLL ADVAPI32.dll=DNADVAPI.DLL
%K%\DN98INST.EXE RENAMEIMPORTS %W%\windowscodecsext.dll ntdll.dll=NTDTA.DLL

rem On Windows the XNA redistributable installs the managed XNA assemblies in the GAC, and
rem some clients (Dawn of the Tiberium Age 15) rely on that. Give those a copy beside the client.
if exist Resources\Microsoft.Xna.Framework.dll goto havexna
if exist Resources\Binaries\XNA\Microsoft.Xna.Framework.dll goto havexna
if not exist %W%\Microsoft_Xna_Framework_dll_GAC goto extractfail
echo Adding the XNA Framework assemblies to Resources ...
copy %W%\Microsoft_Xna_Framework_dll_GAC Resources\Microsoft.Xna.Framework.dll > nul
copy %W%\Microsoft_Xna_Framework_Avatar_dll_GAC Resources\Microsoft.Xna.Framework.Avatar.dll > nul
copy %W%\Microsoft_Xna_Framework_Game_dll_GAC Resources\Microsoft.Xna.Framework.Game.dll > nul
copy %W%\Microsoft_Xna_Framework_GamerServices_dll_GAC Resources\Microsoft.Xna.Framework.GamerServices.dll > nul
copy %W%\Microsoft_Xna_Framework_Graphics_dll_GAC Resources\Microsoft.Xna.Framework.Graphics.dll > nul
copy %W%\Microsoft_Xna_Framework_Input_Touch_dll_GAC Resources\Microsoft.Xna.Framework.Input.Touch.dll > nul
copy %W%\Microsoft_Xna_Framework_Net_dll_GAC Resources\Microsoft.Xna.Framework.Net.dll > nul
copy %W%\Microsoft_Xna_Framework_Storage_dll_GAC Resources\Microsoft.Xna.Framework.Storage.dll > nul
copy %W%\Microsoft_Xna_Framework_Video_dll_GAC Resources\Microsoft.Xna.Framework.Video.dll > nul
copy %W%\Microsoft_Xna_Framework_Xact_dll_GAC Resources\Microsoft.Xna.Framework.Xact.dll > nul
:havexna

echo Installing into Resources ...
call Win98\COPYKIT.BAT Resources
if not exist Resources\Binaries\XNA\NUL goto noxnadir
echo Installing into Resources\Binaries\XNA ...
call Win98\COPYKIT.BAT Resources\Binaries\XNA
:noxnadir

if not exist Syringe.exe goto nosyringe
echo Replacing Syringe.exe with the build that works on Windows 98 ...
%K%\DN98INST.EXE DOWNLOAD https://github.com/11EJDE11/SyringeEx/releases/download/win98-rel32-1/Syringe.exe %D%\SYRINGE.EXE 823b01cc62af0ae6a7662b1ed0138b382b0f82e7
if not exist %D%\SYRINGE.EXE goto dlfail
if not exist Win98\SYRINGE.ORG copy Syringe.exe Win98\SYRINGE.ORG > nul
copy %D%\SYRINGE.EXE Syringe.exe > nul
:nosyringe

if not exist %windir%\SYSTEM\GDIPLUS.DLL echo Warning: GDI+ is not installed. Run .NET 4.8 for Windows 98 setup again.
echo.
echo Done. Start the client with Resources\clientxna.exe
echo After a CnCNet update, run CNCNET98.BAT again.
goto end

:notgame
echo Resources\clientxna.exe is not here. Put CNCNET98.BAT and the Win98 folder in the
echo game folder (the one with the Resources folder) and run it from there.
goto end

:nodotnet
echo .NET Framework 4.8 for Windows 98 is not installed (C:\DOTNET48\DN98INST.EXE is
echo missing). Install it first: https://github.com/11EJDE11/dotnet48-win98
goto end

:olddotnet
echo .NET Framework 4.8 for Windows 98 is out of date. Install the latest release first:
echo https://github.com/11EJDE11/dotnet48-win98
goto end

:nokit
echo The Win98 folder is missing. Copy it into the game folder beside CNCNET98.BAT.
goto end

:dlfail
echo A download failed. Check the internet connection and run CNCNET98.BAT again, or put the
echo files in Win98\DL yourself (see README.md).
goto end

:extractfail
echo Extracting the redistributables failed. Delete Win98\DL and Win98\WORK and try again.
goto end

:end
