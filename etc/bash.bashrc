#!/bin/bash

export LAST_LOCATION_FILE="/home/b/.files/.data/logs/last_location.state.txt"

cd "$(<$LAST_LOCATION_FILE)"

f="/usr/share/bash-completion/bash_completion"

[[ -r "$f" ]] && . "$f"
