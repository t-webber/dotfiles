#!/bin/bash

print() { echo -e "\x1b[33m==== User:$1\x1b[0m"; }

print shellrc

. /del/.dot/cmd/sh/shellrc

print logs

[ -d $LOGS ] && r $LOGS
nd $SLOGS
date >$SLOGS/date
sl=$SLOGS/profile

print sh-startup

. sh_startup &>$sl-sh-startup &

print fix-tty

sudo chmod ugoa+rwx /dev/tty*

print X

xinit $OCFG/xinitrc &>$sl-xi &

disown
