#!/bin/sh 

if [[ -z $ETC ]]; then 
	echo "ETC var not configured"
	exit 1
fi

link() {
	path="$1"
	file="$(echo $path | awk -F/ '{print $NF}')"

	sudo rm -f "$path"
	sudo ln -s "$ETC/$file" "$path"
}

link "/etc/pacman.conf"
