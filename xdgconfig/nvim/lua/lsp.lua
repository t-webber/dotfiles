local root = vim.fs.root(0, { 'pyproject.toml', '.git', 'venv' })
	or vim.fn.getcwd()

local lsps = {

	zig = { cmd = { 'zls' }, filetypes = { 'zig' } },

	py = {
		cmd = { 'pyright-langserver', '--stdio' },
		filetypes = { 'python' },
		root_markers = { 'pyproject.toml', '.venv', 'venv' },
		settings = {
			python = {
				pythonPath = 'python3',
				analysis = {
					typeCheckingMode = 'strict',
					autoSearchPaths = true,
					useLibraryCodeForTypes = true,
					useLibraryCodeForTypes = true,
					reportMissingTypeStubs = false,
					-- 					diagnosticSeverityOverrides = python.diagnostics,
					-- 					extraPaths = python.extraPaths,
				},
			},
			-- 			venvPath = python.venvPath,
			-- 			venv = python.venv,
		},
	},

	tex = {
		cmd = { 'texlab' },
		filetypes = { 'tex' },
	},
	ts = {
		cmd = { 'typescript-language-server', '--stdio' },
		filetypes = {
			'typescript',
			'typescriptreact',
			'javascript',
			'javascriptreact',
		},
		root_dir = vim.fs.dirname(
			vim.fs.find({ 'package.json' }, { upward = true })[1]
		),
	},
	lua = {
		cmd = { 'lua-language-server' },
		filetypes = { 'lua' },
		settings = {
			Lua = {
				runtime = { version = 'LuaJIT' },
				diagnostics = { globals = { 'vim' } },
			},
		},
	},
	rs = {
		cmd = { 'rust-analyzer' },
		filetypes = { 'rust' },
		settings = {
			['rust-analyzer'] = {
				cargo = { features = 'all' },
				check = { command = 'clippy' },
				diagnostics = { styleLints = { enable = true } },
				imports = {
					granularity = { group = 'module' },
					preferNoStd = true,
				},
				inlayHints = {
					bindingModeHints = { enable = true },
					maxLength = nil,
				},
			},
		},
	},
	c = {
		cmd = { 'clangd' },
		filetypes = { 'c', 'cpp' },
	},
	sh = {
		cmd = { 'bash-language-server' },
		filetypes = { 'sh', 'bash' },
	},
	go = {
		cmd = { 'gopls' },
		filetypes = { 'go' },
	},

	-- 	Ruff = {
	-- 		cmd = { 'ruff-lsp' },
	-- 		filetypes = { 'python' },
	-- 	},
}

for name, config in pairs(lsps) do
	vim.lsp.enable(name)
	if config['root_markers'] == nil then config['root_markers'] = {} end
	for _, markers in ipairs({ '.git', '.' }) do
		table.insert(config['root_markers'], marker)
	end
	vim.lsp.config(name, config)
end

local v = require('globals')
local pyfmt = v.pyfmt

vim.api.nvim_create_autocmd('BufWritePre', {
	pattern = '*.py',
	callback = function()
		if vim.g[pyfmt] ~= '' then
			local buf = vim.api.nvim_get_current_buf()
			local filepath = vim.api.nvim_buf_get_name(buf)

			vim.cmd('write')

			vim.fn.system({ 'ruff', 'check', '--fix', filepath })
			vim.fn.system({ 'black', filepath })
			vim.fn.system({ 'isort', filepath })

			vim.cmd('edit!')
		end
	end,
})

return python
