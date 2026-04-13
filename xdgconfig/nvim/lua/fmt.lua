return {

	-----------------
	--- Dedicated ---
	-----------------

	lua = { 'stylua' },
	rust = { 'rustfmt' },
	sql = { 'sql_formatter' },
	python = { 'ruff_fix', 'ruff_format' },
	toml = { 'taplo' },
	go = { 'gofmt' },
	kdl = { 'kdlfmt' },
	zig = { 'zigfmt' },
	xml = { 'tidy' },
	kotlin = { 'ktlint' },

	-------------
	--- Shell ---
	-------------

	sh = { 'shfmt', ln = 'bash' },
	bash = { 'shfmt', ln = 'bash' },
	zsh = { 'shfmt', ln = 'zsh' },

	----------------
	--- Prettier ---
	----------------

	javascript = { '_prettier' },
	javascriptreact = { '_prettier' },
	typescript = { '_prettier' },
	typescriptreact = { '_prettier' },
	html = { '_prettier' },
	css = { '_prettier' },
	json = { '_prettier' },
	jsonc = { '_prettier' },
	markdown = { '_prettier' },
	yaml = { '_prettier' },

	--------------------
	--- Clang Format ---
	--------------------

	-- for .h: vim.g.c_syntax_for_h = 1 (see settings.lua)
	c = { 'clang-format' },
	cpp = { 'clang-format' },
}
