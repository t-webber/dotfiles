#!/bin/sh

export FILES=/home/b/.files
export DEVICE=acer
export XCLIP='/bin/xclip -selection c'

f="/usr/share/bash-completion/bash_completion"

[ -r "$f" ] && . "$f"

. "$FILES/dotfiles/cmd/sh/shellrc"
