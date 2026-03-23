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

	javascript = { 'pret' },
	javascriptreact = { 'pret' },
	typescript = { 'pret' },
	typescriptreact = { 'pret' },
	html = { 'pret' },
	css = { 'pret' },
	json = { 'pret' },
	jsonc = { 'pret' },
	markdown = { 'pret' },

	--------------------
	--- Clang Format ---
	--------------------

	-- for .h: vim.g.c_syntax_for_h = 1 (see settings.lua)
	c = { 'clang-format' },
	cpp = { 'clang-format' },
}
