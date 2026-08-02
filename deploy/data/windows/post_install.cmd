sc stop AmneziaWGTunnel$AbstractumVPN
sc delete AmneziaWGTunnel$AbstractumVPN
taskkill /IM "AbstractumVPN-service.exe" /F
taskkill /IM "AbstractumVPN.exe" /F
exit /b 0
