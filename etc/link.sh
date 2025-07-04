#!/bin/sh

if [ -z "$ETC" ]; then
	echo "ETC var not configured"
	exit 1
fi

way="$1"

if [ -z "$1" ]; then
	rm -rf "$ETC/etc"
fi

copy() {
	echo "$1 => $2"
	sudo cp -r "$1" "$2"
}

link() {
	root_path="$1"
	link_path="$ETC$1"
	save_path="$ETC/save$1"
	link_folder="$(echo "$link_path" | awk -F/ '{OFS=FS; $NF=""; print}')"
	save_folder="$(echo "$save_path" | awk -F/ '{OFS=FS; $NF=""; print}')"

	if [ -z "$way" ]; then
		mkdir -p "$link_folder"
		copy "$root_path" "$link_path"
		sudo chown "$USER:$USER" "$link_path" -R
		sudo chmod u+rw "$link_path" -R
	else
		mkdir -p "$save_folder"
		copy "$root_path" "$save_path"
		copy "$link_path" "$root_path"
		sudo chown "root:root" "$root_path" -R
	fi
}

link "/etc/profile"
link "/etc/bash.bashrc"

link "/etc/hostname"

link "/etc/vconsole.conf"
link "/etc/mkinitcpio.conf"
link "/etc/locale.gen"
link "/etc/locale.conf"
link "/etc/inputrc"

link "/etc/X11/xorg.conf.d"
link "/etc/X11/xinit/xinitrc"
link "/etc/iwd"

link "/etc/pacman.conf"
link "/etc/makepkg.conf"
link "/etc/makepkg.conf.d"
