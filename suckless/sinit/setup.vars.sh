###############
### FOLDERS ###
###############

### DEPTH 1 ###

export FILES=/del

### DEPTH 2 ###

export DATA="$FILES/.data"
export WASTE="$FILES/.waste"
export DOT="$FILES/.dot"
export APPS="$FILES/.apps"
export DEV="$FILES/.dev"
export SECRET="$FILES/.secret"
export STUDY="$FILES/.study"
export BIN="$FILES/.bin"
export BLOB="$FILES/.blob"
export WORK="$FILES/.work"

### DEPTH 3 ###

export CMD="$DOT/cmd"
export XCFG="$DOT/xdgconfig"
export XDG_CONFIG_HOME="$XCFG"
export ETC="$DOT/etc"
export OCFG="$DOT/otherconfig"

export LOGS="$DATA/logs"
export XDG_DATA_HOME="$DATA/.share"
export XDG_CACHE_HOME="$DATA/.cache"
export XDG_STATE_HOME="$DATA/.state"
export XDG_RUNTIME_DIR="$DATA/rt"

############
### PATH ###
############

path() {
        if [ -d "$1" ]; then
                PATH+=":$1"
        fi
}

export CARGO_HOME="$DATA/cargo"
export BUN_INSTALL="$DATA/bun"
export ANDROID_HOME=/opt/android-sdk/

PATH=""
path "$CMD/bin"
path "/bin"
path "$BIN"
path "$CMD/old"
path "$CMD/sh"
path "$CARGO_HOME/bin"
path "$BUN_INSTALL/bin"
path "$APPS/flutter/bin"
path "$ANDROID_HOME/emulator"
path "$ANDROID_HOME/platform-tools"
path "$ANDROID_HOME/cmdline-tools/latest/bin/"
path "$ANDROID_HOME/build-tools/36.0.0/"
export PATH

##############
### NEEDED ###
##############

export SHELL=/bin/bash
export EDITOR=/bin/nvim
