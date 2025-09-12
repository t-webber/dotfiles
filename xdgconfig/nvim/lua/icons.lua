return {
	{ short = 'r', icon = ' ', var = os.getenv('RUST_SRC') },
	{ short = 'n', icon = ' ', var = os.getenv('XCFG') .. '/nvim/lua' },
	{ short = 'v', icon = ' ', var = os.getenv('DEV') },
	{ short = 'x', icon = ' ', var = os.getenv('XCFG') },
	{ short = 'o', icon = ' ', var = os.getenv('DOT') },
	{ short = 'f', icon = '', var = os.getenv('FILES') },
	{ short = 't', icon = '󰃖 ', var = os.getenv('WORK') },
	{ short = 'w', icon = ' ', var = os.getenv('WASTE') },
}
