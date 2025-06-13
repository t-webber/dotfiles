#!/bin/sh

if [ -z "$ETC" ]; then
	echo "ETC var not configured"
	exit 1
fi

rm -rf "$ETC/etc"

link() {
	root_path="$1"
	link_path="$ETC$1"
	link_folder="$(echo "$link_path" | awk -F/ '{OFS=FS; $NF=""; print}')"

	mkdir -p "$link_folder"
	sudo cp -r "$root_path" "$link_path"
	sudo chown "$USER:$USER" "$link_path" -R
	sudo chmod a+rwx "$link_path" -R

}

link "/etc/profile"
link "/etc/bash.bashrc"

link "/etc/hostname"

link "/etc/vconsole.conf"
link "/etc/locale.gen"
link "/etc/locale.conf"
link "/etc/inputrc"

link "/etc/X11/xorg.conf.d"
link "/etc/X11/xinit/xinitrc"
link "/etc/iwd"

link "/etc/pacman.conf"
link "/etc/pacman.d/mirrorlist"
link "/etc/makepkg.conf"
link "/etc/makepkg.conf.d"
