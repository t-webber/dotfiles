#!/bin/sh

if [ -z "$ETC" ]; then
        echo "ETC var not configured"
        exit 1
fi

vb="$1"

rm -rf "$ETC/etc" "$ETC/arch" "$ETC/cargo"

link() {
        root_path="$1"
        link_path="$ETC$1"
        parent_path="$(dirname "$link_path")"
        mkdir -p "$parent_path"
        [ -n "$vb" ] && l "%2$root_path => $link_path ($parent_path)"
        rm -rf "$link_path"
        sudo cp -r "$root_path" "$link_path"
        sudo chown "$USER:$USER" "$link_path" -R
        sudo chmod u+rw "$link_path" -R
}

link "/etc/hostname"

link "/etc/vconsole.conf"
link "/etc/mkinitcpio.conf"
link "/etc/locale.gen"
link "/etc/locale.conf"
link "/etc/inputrc"

link "/etc/pacman.conf"
link "/etc/makepkg.conf"
link "/etc/makepkg.conf.d"

cargo install --list | grep -v '^ ' | awk '{print $1}' | tr '\n' ' ' >"$ETC/cargo"
pacman -Qetq >"$ETC/arch"
