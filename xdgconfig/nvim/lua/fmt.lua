local python = require('lsp')

if os.getenv('DEVICE') == 'v' then
	python = 'yapf'
else
	python = 'ruff'
end

return {

	-----------------
	--- Dedicated ---
	-----------------

	lua = { 'stylua' },
	rust = { 'rustfmt' },
	sql = { 'sql_formatter' },
	python = { python },
	toml = { 'taplo' },
	go = { 'gofmt' },

	-------------
	--- Shell ---
	-------------

	sh = { 'shfmt', ln = 'bash' },
	bash = { 'shfmt', ln = 'bash' },

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
