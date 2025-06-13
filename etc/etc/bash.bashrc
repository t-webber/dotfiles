#!/bin/sh

export DEVICE=acer
export FILES=/home/b/.files

f="/usr/share/bash-completion/bash_completion"

[ -r "$f" ] && . "$f"

. "$FILES/dotfiles/cmd/sh/shellrc"

