@echo off
rem Full uninstall: the machine should look as if AbstractumVPN had never been
rem installed. Everything below is named after this product only - an AmneziaVPN
rem installed alongside keeps its own services, settings and credentials.
rem
rem Deliberately NOT removed: the service "AmneziaVPNSplitTunnel", left behind by
rem builds of ours from before the rename. That name is indistinguishable from a
rem live Amnezia installation's own service, so deleting it here would break the
rem neighbour. Remove it by hand if Amnezia is not installed.
rem
rem This runs as the uninstalling user: another user's profile keeps its copy of
rem the settings.

set "ORG_DIR=%AppData%\AbstractumVPN"
set "USER_APP_DIR=%ORG_DIR%\AbstractumVPN"
set "LOCAL_DIR=%LocalAppData%\AbstractumVPN"
set "SYS_APP_DIR=%ProgramData%\AbstractumVPN"

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

exit /b 0
