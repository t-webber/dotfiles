#!/bin/sh 

if [[ -z $ETC ]]; then 
	echo "ETC var not configured"
	exit 1
fi

link() {
	path="$1"
	if [[ -z $2 ]]; then 
		file="$(echo $path | awk -F/ '{print $NF}')"
	else
		file="$2"
	fi

	sudo rm -f "$path"
	sudo ln -s "$ETC/$file" "$path"
}

link "/etc/profile"
link "/etc/bash.bashrc"

link "/etc/hostname"

link "/etc/vconsole.conf"
link "/etc/locale.gen"
link "/etc/locale.conf"
link "/etc/inputrc"

link "/etc/resolv.conf"
link "/etc/iwd/main.conf" "iwd.conf"

link "/etc/pacman.conf"
link "/etc/pacman.d/mirrorlist" 
link "/etc/makepkg.conf"
link "/etc/makepkg.conf.d/rust.conf" "makepkg-rust.conf"

link "/etc/X11/xorg.conf.d/50-libinput.conf" 

