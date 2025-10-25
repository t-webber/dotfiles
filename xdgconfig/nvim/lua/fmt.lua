local python = require('lsp')
local yapf
if python.venv_bin == nil then
	yapf = 'yapf'
else
	yapf = python.venv_bin .. 'yapf'
end

return {

	-----------------
	--- Dedicated ---
	-----------------

	lua = { 'stylua' },
	rust = { 'rustfmt' },
	sql = { 'sql_formatter' },
	python = { yapf },
	toml = { 'taplo' },
	go = { 'gofmt' },
	kdl = { 'kdlfmt' },

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
