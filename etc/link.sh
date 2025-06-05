#!/bin/sh

if [ -z "$FILES" ]; then
    echo "FILES var not configured"
    exit 1
fi

link() {
    root_path="$1"
    link_path="$FILES$1"

    sudo mv "$root_path" "$link_path"
    ln -s "$link_path" "$root_path"

}

link "/etc/profile"
link "/etc/bash.bashrc"

link "/etc/hostname"

link "/etc/vconsole.conf"
link "/etc/locale.gen"
link "/etc/locale.conf"
link "/etc/inputrc"

link "/etc/X11/xorg.conf.d"
link "/etc/iwd"

link "/etc/pacman.conf"
link "/etc/pacman.d"
link "/etc/makepkg.conf"
link "/etc/makepkg.conf.d"
