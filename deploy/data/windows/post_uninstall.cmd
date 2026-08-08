@echo off
rem Full uninstall: the machine should look as if AbstractumVPN had never been
rem installed. Everything below is named after this product only - an AmneziaVPN
rem installed alongside keeps its own services, settings and credentials.
rem
rem The one service with an ambiguous name, "AmneziaVPNSplitTunnel", is handled
rem near the end of this file - by what its driver file is, not by what it is
rem called.
rem
rem This runs as the uninstalling user: another user's profile keeps its copy of
rem the settings.

set "ORG_DIR=%AppData%\AbstractumVPN"
set "USER_APP_DIR=%ORG_DIR%\AbstractumVPN"
set "LOCAL_DIR=%LocalAppData%\AbstractumVPN"
set "SYS_APP_DIR=%ProgramData%\AbstractumVPN"

rem The directory this script is running from, i.e. the installation directory.
rem Needed in two places below, so it is worked out once here.
set "INSTALL_DIR=%~dp0"
if "%INSTALL_DIR:~-1%"=="\" set "INSTALL_DIR=%INSTALL_DIR:~0,-1%"
for %%I in ("%INSTALL_DIR%") do set "INSTALL_LEAF=%%~nxI"

timeout /t 1

rem --- Services -------------------------------------------------------------
rem The tunnel service prefix "AmneziaWGTunnel$" is dictated by the amneziawg
rem driver; the part after "$" is ours and is what separates us from Amnezia.
sc stop AbstractumVPN-service
sc delete AbstractumVPN-service
sc stop AmneziaWGTunnel$AbstractumVPN
sc delete AmneziaWGTunnel$AbstractumVPN
rem Registered by the split tunnel driver. The client deletes it in a destructor,
rem which the force kill below skips - so it is deleted here too.
sc stop AbstractumVPNSplitTunnel
sc delete AbstractumVPNSplitTunnel

taskkill /IM "AbstractumVPN-service.exe" /F
taskkill /IM "AbstractumVPN.exe" /F

rem --- IKEv2 phonebook entry ------------------------------------------------
rem Removed by name rather than by deleting rasphone.pbk, which holds every
rem other VPN connection the user has.
powershell -NoProfile -ExecutionPolicy Bypass -Command "Remove-VpnConnection -Name 'AbstractumVPN IKEv2' -Force -ErrorAction SilentlyContinue" >nul 2>&1

rem --- Stored credentials ---------------------------------------------------
rem The settings encryption key lives in Credential Manager under
rem "AbstractumVPN-Keychain/<tag>". Only targets whose name starts with that
rem string are touched.
powershell -NoProfile -ExecutionPolicy Bypass -Command "& { (cmdkey /list) -match 'AbstractumVPN-Keychain' | ForEach-Object { if ($_ -match '(AbstractumVPN-Keychain\S*)') { cmdkey /delete:$($Matches[1]) | Out-Null } } }" >nul 2>&1

rem --- Settings and autostart in the registry -------------------------------
rem QSettings stores under HKCU\Software\<organization>\<application>.
reg delete "HKCU\Software\AbstractumVPN" /f >nul 2>&1
reg delete "HKCU\Software\Microsoft\Windows\CurrentVersion\Run" /v AbstractumVPN /f >nul 2>&1

rem --- Data directories -----------------------------------------------------
rem Roaming holds the server list, logs and the OpenVPN config; Local holds the
rem tunnel configuration; ProgramData holds the service log.
if exist "%USER_APP_DIR%" rmdir /S /Q "%USER_APP_DIR%"
rd "%ORG_DIR%" 2>nul
if exist "%LOCAL_DIR%" rmdir /S /Q "%LOCAL_DIR%"
if exist "%SYS_APP_DIR%" rmdir /S /Q "%SYS_APP_DIR%"

rem --- Driver directory created by the tunnel -------------------------------
rem "%ProgramFiles%\AmneziaWG" is not put there by our installer: the tunnel
rem library creates it on the first connection for the driver's ring log. On a
rem machine without Amnezia it is therefore ours, and leaving it behind means
rem leaving a directory named after another product.
rem
rem It is removed only when nothing suggests Amnezia is installed. The service
rem name comes from the same code we inherited - APPLICATION_NAME + "-service" -
rem so their build registers "AmneziaVPN-service" exactly the way ours registers
rem "AbstractumVPN-service".
set "AWG_DIR=%ProgramFiles%\AmneziaWG"
set "AMNEZIA_PRESENT="
if exist "%ProgramFiles%\AmneziaVPN" set "AMNEZIA_PRESENT=1"
if exist "%ProgramFiles(x86)%\AmneziaVPN" set "AMNEZIA_PRESENT=1"
if exist "%AppData%\AmneziaVPN" set "AMNEZIA_PRESENT=1"
sc query AmneziaVPN-service >nul 2>&1 && set "AMNEZIA_PRESENT=1"

rem Plain gotos rather than if/else blocks: nothing in this file is exercised by
rem CI, so the parsing has to be as dull as possible.
if defined AMNEZIA_PRESENT goto :keep_driver_dir
if exist "%AWG_DIR%" rmdir /S /Q "%AWG_DIR%"
:keep_driver_dir

rem --- Split tunnel service from our builds before the rename ---------------
rem "AmneziaVPNSplitTunnel" is the name our own builds registered until
rem 07.08.2026. A live Amnezia registers a service under exactly that name too,
rem so the name proves nothing - but the driver file behind it does. It is a
rem kernel driver service, and its ImagePath points at the installation
rem directory of whichever client created it.
rem
rem So: remove it only when its driver lives in the directory being uninstalled
rem right now. On a machine where both clients are installed this still answers
rem correctly, which "is Amnezia present" cannot - and that machine is the only
rem kind that has this leftover at all.
rem
rem ImagePath is read straight from the registry rather than parsed out of
rem "sc qc": the registry value has no human-readable formatting to depend on.
set "LEGACY_ST=AmneziaVPNSplitTunnel"
set "ABS_INSTALL_DIR=%INSTALL_DIR%"
powershell -NoProfile -ExecutionPolicy Bypass -Command "$key = 'HKLM:\SYSTEM\CurrentControlSet\Services\' + $env:LEGACY_ST; $image = (Get-ItemProperty -Path $key -Name ImagePath -ErrorAction SilentlyContinue).ImagePath; if ($image -and $image -like ('*' + $env:ABS_INSTALL_DIR + '*')) { exit 0 }; exit 1" >nul 2>&1
if errorlevel 1 goto :keep_legacy_service

sc stop %LEGACY_ST%
sc delete %LEGACY_ST%
:keep_legacy_service

rem --- The installation directory itself ------------------------------------
rem maintenancetool.exe cannot delete itself while it is running, so it hands
rem that to a step of its own after exit - and that step is NOT elevated. Under
rem Program Files it fails without a word, leaving a 32 MB uninstaller behind
rem for a product that is gone.
rem
rem This script is elevated, so it starts a detached waiter: wait for
rem maintenancetool to exit, then remove the directory. The name is checked
rem first - this is a recursive delete, and the path comes from wherever the
rem script happens to be running.
if /i not "%INSTALL_LEAF%"=="AbstractumVPN" goto :done

start "" /b powershell -NoProfile -ExecutionPolicy Bypass -WindowStyle Hidden -Command "Set-Location $env:SystemRoot; Get-Process -Name maintenancetool -ErrorAction SilentlyContinue | Wait-Process -Timeout 300 -ErrorAction SilentlyContinue; Start-Sleep -Seconds 3; Remove-Item -LiteralPath '%INSTALL_DIR%' -Recurse -Force -ErrorAction SilentlyContinue" >nul 2>&1

:done
exit /b 0
