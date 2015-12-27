#!/bin/bash

# This script makes it easier to deploy changes to the fvwm95rc file.
# To use this file. set the $ACCOUNTNAME variable to the user account associated
# with your FVWM95 testing user account. The script will automatically replace the
# fvwm95rc file in that account

# When doing fvwm95 development, work on the system.fvwm95rc.in file located here instead of the
# user's local fvwm95rc. This will allow the changes you make be available for future users.

# Set account name variable here:
ACCOUNTNAME="fvwmuser"

PWD=$(pwd)

cp $PWD/system.fvwm95rc.in /home/$ACCOUNTNAME/.fvwm95rc
chown $ACCOUNTNAME /home/$ACCOUNTNAME/.fvwm95rc

