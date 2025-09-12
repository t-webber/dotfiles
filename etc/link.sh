#!/bin/sh

if [ -z "$ETC" ]; then
	echo "ETC var not configured"
	exit 1
fi

rm -rf "$ETC/etc" "$ETC/arch" "$ETC/cargo"
echo "Deleted" "$ETC/etc" "$ETC/arch" "$ETC/cargo"

copy() {
	echo "$1 => $2"
	sudo cp -r "$1" "$2"
}

parent_folder() {
	parent="$(echo "$1" | awk -F/ '{OFS=FS; $NF=""; print}')"
	if [ -z "$parent" ]; then
		:
	else
		if [ -z "$2" ]; then
			echo "mkdir $parent"
			mkdir -p "$parent"
		else
			echo "sudo mkdir $parent"
			sudo mkdir -p "$parent"
		fi
	fi
}

link() {
	root_path="$1"
	link_path="$ETC$1"
	save_path="$ETC/save$1"

	root_folder="$(parent_folder $root_path !)"
	link_folder="$(parent_folder $link_path)"

	if [ -e "$root_path" ]; then
		copy "$root_path" "$link_path"
	fi
	sudo chown "$USER:$USER" "$link_path" -R
	sudo chmod u+rw "$link_path" -R
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

cargo install --list | grep -v '^ ' | awk '{print $1}' | tr '\n' ' ' >"$ETC/cargo"
pacman -Qentq >"$ETC/arch"
