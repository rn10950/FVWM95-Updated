#!/bin/bash

# This shell script updates the XCLass taskbatr and associated rc files
# Set FVWM95 user account as the $ACCOUNTNAME variable.

# You must run this script as superuser (root) and FVWM95 must already be installed in its default location

ACCOUNTNAME="fvwmuser"

PWD=$(pwd)

# ===== copy binaries ======
mkdir -p /usr/local/xclass/apps/taskbar/
cp $PWD/taskbar /usr/local/lib/X11/fvwm95/XClassTaskbar
cp $PWD/plugins/battstat/battstat.so /usr/local/xclass/apps/taskbar/
cp $PWD/plugins/clock/clock.so /usr/local/xclass/apps/taskbar/
cp $PWD/plugins/cpuload/cpuload.so /usr/local/xclass/apps/taskbar/
cp $PWD/plugins/icon/icon.so /usr/local/xclass/apps/taskbar/
cp $PWD/plugins/lockfile/lockfile.so /usr/local/xclass/apps/taskbar/
cp $PWD/plugins/mailcheck/mailcheck.so /usr/local/xclass/apps/taskbar/

# ===== start replacing =====
cp $PWD/EXAMPLE.taskbarrc /home/$ACCOUNTNAME/.taskbarrc
chown $ACCOUNTNAME /home/$ACCOUNTNAME/.taskbarrc
