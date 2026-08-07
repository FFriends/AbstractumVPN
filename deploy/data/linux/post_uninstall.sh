#!/bin/bash

APP_NAME=AbstractumVPN
# Must equal ORGANIZATION_NAME in version.h - Qt builds every user-data path as
# <base>/$ORG_NAME/$APP_NAME. This said "AbstractumVPN.ORG" until 07.08.2026,
# a name the application never writes to, so nothing under the user's home was
# ever actually removed.
ORG_NAME=AbstractumVPN
LOG_FOLDER=/var/log/$APP_NAME
LOG_FILE="$LOG_FOLDER/post-uninstall.log"
APP_PATH=/opt/$APP_NAME

if ! test -f $LOG_FILE; then
	touch $LOG_FILE
fi

date >> $LOG_FILE
echo "Uninstall Script started" >> $LOG_FILE
sudo killall -9 $APP_NAME 2>> $LOG_FILE

if command -v steamos-readonly &> /dev/null; then
	sudo steamos-readonly disable >> $LOG_FILE
	echo "steamos-readonly disabled" >> $LOG_FILE
fi

ls /opt/AbstractumVPN/client/lib/* | while IFS=: read -r dir; do
	sudo unlink $dir  >> $LOG_FILE
done

if sudo systemctl is-active --quiet $APP_NAME; then
	sudo systemctl stop $APP_NAME >> $LOG_FILE
fi

if sudo systemctl is-enabled --quiet $APP_NAME; then
	sudo systemctl disable $APP_NAME >> $LOG_FILE
fi

if test -f /etc/systemd/system/$APP_NAME.service; then
	sudo rm -rf /etc/systemd/system/$APP_NAME.service >> $LOG_FILE
fi

if test -f $APP_PATH; then
        sudo rm -rf $APP_PATH >> $LOG_FILE
fi

if test -f /usr/sbin/$APP_NAME; then
        sudo rm -f /usr/sbin/$APP_NAME >> $LOG_FILE
fi

if test -f /usr/bin/$APP_NAME; then
        sudo rm -f /usr/bin/$APP_NAME >> $LOG_FILE
fi

if test -f /usr/local/bin/$APP_NAME; then
        sudo rm -f /usr/local/bin/$APP_NAME >> $LOG_FILE
fi

if test -f /usr/local/sbin/$APP_NAME; then
        sudo rm -f /usr/local/sbin/$APP_NAME >> $LOG_FILE
fi

if test -f /usr/share/applications/$APP_NAME.desktop; then
	sudo rm -f /usr/share/applications/$APP_NAME.desktop >> $LOG_FILE

fi

if test -f /usr/share/pixmaps/$APP_NAME.png; then
	sudo rm -f /usr/share/pixmaps/$APP_NAME.png >> $LOG_FILE

fi

### Remove the service log file (keep post-uninstall.log)
if test -f "$LOG_FOLDER/AbstractumVPN-service.log"; then
    sudo rm -f "$LOG_FOLDER/AbstractumVPN-service.log" >> $LOG_FILE 2>&1
fi

### Remove everything the application wrote into the user's home, so the machine
### looks as if AbstractumVPN had never been installed. Every path below is named
### after this product only - an Amnezia Client installed alongside keeps its own
### settings, servers and logs.
###
### Current user only: another account keeps its copy.
TARGET_HOME="$HOME"
if [ -n "$SUDO_USER" ] && [ "$SUDO_USER" != "root" ]; then
    TARGET_HOME=$(getent passwd "$SUDO_USER" | cut -d: -f6)
fi

### Server list, logs and the OpenVPN config
if test -d "$TARGET_HOME/.local/share/$ORG_NAME"; then
    rm -rf "$TARGET_HOME/.local/share/$ORG_NAME" >> $LOG_FILE 2>&1
    echo "Removed user data under .local/share/$ORG_NAME" >> $LOG_FILE
fi

### Settings written by QSettings
if test -d "$TARGET_HOME/.config/$ORG_NAME"; then
    rm -rf "$TARGET_HOME/.config/$ORG_NAME" >> $LOG_FILE 2>&1
    echo "Removed settings under .config/$ORG_NAME" >> $LOG_FILE
fi

### Autostart entry, written when the user enables "launch on login"
if test -f "$TARGET_HOME/.config/autostart/$APP_NAME.desktop"; then
    rm -f "$TARGET_HOME/.config/autostart/$APP_NAME.desktop" >> $LOG_FILE 2>&1
    echo "Removed autostart entry" >> $LOG_FILE
fi

### Settings encryption key, if a secret service is in use. Absent on machines
### without one - on Linux the settings are not encrypted anyway.
if command -v secret-tool &> /dev/null; then
    secret-tool clear service "$APP_NAME-Keychain" >> $LOG_FILE 2>&1
fi

if command -v steamos-readonly &> /dev/null; then
	sudo steamos-readonly enable >> $LOG_FILE
	echo "steamos-readonly enabled" >> $LOG_FILE
fi

date >> $LOG_FILE
echo "Service after uninstall status:" >> $LOG_FILE
sudo systemctl status $APP_NAME >> $LOG_FILE
date >> $LOG_FILE
echo "Script finished" >> $LOG_FILE
