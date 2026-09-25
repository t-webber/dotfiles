#!/bin/bash

print() { echo -e "\x1b[33m├─── WebberOS:$1\x1b[0m"; }
here=$(dirname $0)

#####
print startup
#####

startup() {
        l / >$LOGS/l-lwd

        net startup &>$LOGS/l-net &

        sleep 30

        "$ETC/link.sh" &>$LOGS/l-link

        while ! getent hosts example.com &>>$LOGS/l-net; do
                sleep 3
        done

        sudo pkill udhcpc

        # tldr --update
        # clipcat
        # bur

        # yay -Rcns $(yay -Qdtq) --noconfirm &>"$LOGS/l-pacman"
        # yay -Fy &>>$LOGS/l-pacman
        # sudo pkgfile --update &>>$LOGS/l-pacman
}

startup | tee $LOGS/l-startup &

#####
print fix-tty
#####

sudo chmod ugoa+rwx /dev/tty*

#####
print xserver
#####

xinit $here/setup.xinit.sh -- -logfile "$LOGS/x-xorg" &>$LOGS/x-xinit &

disown
