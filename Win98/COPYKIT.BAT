@echo off
rem Called by CNCNET98.BAT: puts the client's native DLLs in folder %1 and makes that
rem folder's Microsoft.Xna.Framework.dll use the Windows 98 replacements.
set T=%1
copy Win98\WORK\XnaNative.dll %T% > nul
copy Win98\WORK\d3dx9_41.dll %T% > nul
copy Win98\WORK\X3DAudio1_7.dll %T% > nul
copy Win98\WORK\MSVCR100.dll %T% > nul
copy Win98\WORK\MSVCP100.dll %T% > nul
copy Win98\WORK\windowscodecs.dll %T% > nul
copy Win98\WORK\windowscodecsext.dll %T% > nul
copy Win98\WORK\wmphoto.dll %T% > nul
copy Win98\WORK\photometadatahandler.dll %T% > nul
copy Win98\OLET3.DLL %T% > nul
copy Win98\NTDTA.DLL %T% > nul
copy Win98\XAudio2_6.dll %T% > nul
rem .NET for Windows 98 installs these in C:\WINDOWS\SYSTEM. An old copy here (from an
rem earlier manual setup) would be loaded instead and can lack functions the Framework needs.
for %%f in (DNKERNEL.DLL DNADVAPI.DLL CORKEL32.DLL CORADV32.DLL BCRYPT.DLL W2_32W.DLL TLS98.DLL WINHTTP.DLL) do if exist %T%\%%f del %T%\%%f
if exist %T%\Microsoft.Xna.Framework.dll C:\DOTNET48\DN98INST.EXE RENAMEIMPORTS %T%\Microsoft.Xna.Framework.dll KERNEL32.dll=DNKERNEL.DLL CORKEL32.DLL=DNKERNEL.DLL ole32.dll=OLET3.DLL
