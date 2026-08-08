local e = os.getenv

return {
        { short = 'a', icon = '󰀻 ', val = e('APPS') },
        { short = 'b', icon = '󱠦 ', val = e('BLOB') },
        { short = 'd', icon = ' ', val = e('DEV') },
        { short = 'f', icon = ' ', val = e('FILES') },
        { short = 'e', icon = ' ', val = '/etc' },
        { short = 'h', icon = '󰋜 ', val = e('HOME') },
        { short = 'i', icon = ' ', val = e('BIN') },
        { short = 'l', icon = ' ', val = e('LOGS') },
        { short = 'm', icon = ' ', val = e('CMD') },
        { short = 'n', icon = ' ', val = e('XCFG') .. '/nvim/lua' },
        { short = 'o', icon = '󰃖 ', val = e('WORK') },
        { short = 'p', icon = ' ', val = '/tmp' },
        { short = 'r', icon = ' ', val = e('RUST_SRC') },
        { short = 's', icon = ' ', val = e('SECRET') },
        { short = 't', icon = '󰆼 ', val = e('DATA') },
        { short = 'u', icon = '󱊟 ', val = '/boot/efi' },
        { short = 'v', icon = '󰫧 ', val = '/var' },
        { short = 'w', icon = ' ', val = e('WASTE') },
        { short = 'x', icon = ' ', val = e('XDG_CONFIG_HOME') },
        { short = '.', icon = ' ', val = e('DOT') },
}
