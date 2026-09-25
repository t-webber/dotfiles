#!/bin/bash

export GTK_THEME=Adwaita:dark
export PASSWORD_STORE_BACKEND=plain
export CHROME_SECRET_STORE=gnome
export XCURSOR_THEME=HUD_Machine_Red_Cursors
export XCURSOR_SIZE=50

tp() {
        xinput set-prop "ELAN0524:00 04F3:3215 Touchpad" "$@"
}

# tap to click
tp "libinput Tapping Enabled" 1
# 2 fingers for right-click
tp "libinput Click Method Enabled" 1 0
# inverse scrolling
tp "ibinput Natural Scrolling Enabled" 1

exec bspwm &>$LOGS/x-bspwm
