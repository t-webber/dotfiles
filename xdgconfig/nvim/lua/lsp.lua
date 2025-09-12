local python

if os.getenv('DEVICE') == 'v' then
	local venv_bin = os.getenv('VENV_BIN')
	local venv = os.getenv('VENV')
	python = {
		venv = venv,
		bin = venv_bin,
		venvPath = venvPath,
		diagnostics = {
			reportUnknownVariableType = false,
			reportUnknownParameterType = false,
			reportMissingTypeStubs = false,
		},
		extraPaths = {
			'tools',
			os.getenv('UNIVENT'),
		},
	}
else
	python = { venv = nil, venvPath = nil, bin = '', diagnostics = {} }
end

local lsps = {

	zig = { cmd = { 'zls' }, filetypes = { 'zig' } },

	py = {
		cmd = { python.bin .. 'pyright-langserver', '--stdio' },
		filetypes = { 'python' },
		settings = {
			python = {
				pythonPath = python.bin .. 'python',
				analysis = {
					-- 					typeCheckingMode = "strict",
					autoSearchPaths = true,
					useLibraryCodeForTypes = true,
					diagnosticSeverityOverrides = python.diagnostics,
					extraPaths = python.extraPaths,
				},
			},
			venvPath = python.venvPath,
			venv = python.venv,
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
		filetypes = { 'c', 'cpp', 'h', 'hpp' },
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
