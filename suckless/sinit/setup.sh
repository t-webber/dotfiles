#!/bin/bash

#################################
########## DEFINITIONS ##########
#################################

first=0
here=$(dirname $0)
tab=$(printf "%115s\n" " ")

print() {
        echo -en "\x1b[33m"
        if [ $first = 0 ]; then
                echo -n "┌"
                first=1
        else
                echo -n "├"
        fi
        echo -e "─── WebberOS:$1\x1b[0m"
}

################################
############ BANNER ############
################################

setfont

printf '\x1b[33m\n\n\n\n\n'
sed "s/^/$tab/" "$here/ascii.current"
printf '\n\n\n\n\n\x1b[0m'

#################################
############# STEPS #############
#################################

#####
print environment
#####

. "$here/setup.vars.sh"

#####
print logging
#####

old="$LOGS.old"
[ -d $old ] && rm -rf $old
[ -d $LOGS ] && mv $LOGS $old
mkdir -p $LOGS
chmod ugoa+rwx -R $LOGS
date >$LOGS/i-date

#####
print stage1
#####

"$here/setup.stage1.sh" &>$LOGS/i-stage1

#####
print agetty
#####

for x in 2; do
        agetty --noclear "tty$x" linux &
done

#####
print user
#####

runuser -u b -- "$here/setup.user.sh" &

######
disown
######
