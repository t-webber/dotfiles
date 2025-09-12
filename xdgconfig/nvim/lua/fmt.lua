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

	-------------
	--- Shell ---
	-------------

	sh = { 'shfmt', ln = 'bash' },
	bash = { 'shfmt', ln = 'bash' },
	zsh = { 'shfmt', ln = 'zsh' },

	----------------
	--- Prettier ---
	----------------

	javascript = { 'prettier' },
	javascriptreact = { 'prettier' },
	typescript = { 'prettier' },
	typescriptreact = { 'prettier' },
	html = { 'prettier' },
	css = { 'prettier' },
	json = { 'prettier' },
	jsonc = { 'prettier' },
	markdown = { 'prettier' },

	--------------------
	--- Clang Format ---
	--------------------

	-- for .h: vim.g.c_syntax_for_h = 1 (see settings.lua)
	c = { 'clang-format' },
	cpp = { 'clang-format' },
}
