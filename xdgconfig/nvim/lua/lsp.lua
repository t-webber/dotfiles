local python

if os.getenv('DEVICE') == 'v' then
	local venvPath = os.getenv('VENV_PATH')
	local venv = os.getenv('VENV')
	local fullPath = venvPath .. '/' .. venv .. '/bin/'
	python = {
		venv = venv,
		fullPath = fullPath,
		venvPath = venvPath,
		diagnostics = {
			reportUnknownVariableType = false,
			reportUnknownParameterType = false,
			reportMissingTypeStubs = false,
		},
		extraPaths = {
			os.getenv('UNIVENT'),
			os.getenv('TOOLS'),
		},
	}
else
	python = { venv = nil, venvPath = nil, fullPath = '', diagnostics = {} }
end

local lsps = {
	Latex = {
		cmd = { 'texlab' },
		filetypes = { 'tex' },
	},
	Typescript = {
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
	Lua = {
		cmd = { 'lua-language-server' },
		filetypes = { 'lua' },
		settings = {
			Lua = {
				runtime = { version = 'LuaJIT' },
				diagnostics = { globals = { 'vim' } },
			},
		},
	},
	Rust = {
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
	Clang = {
		cmd = { 'clangd' },
		filetypes = { 'c', 'cpp', 'h', 'hpp' },
	},
	Bash = {
		cmd = { 'bash-language-server' },
		filetypes = { 'sh', 'bash' },
	},
	Go = {
		cmd = { 'gopls' },
		filetypes = { 'go' },
	},
	Pyright = {
		cmd = { python.fullPath .. 'pyright-langserver', '--stdio' },
		filetypes = { 'python' },
		settings = {
			python = {
				pythonPath = python.fullPath .. 'python',
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

	Ruff = {
		cmd = { 'ruff-lsp' },
		filetypes = { 'python' },
	},
}

for name, config in pairs(lsps) do
	vim.lsp.enable(name)
	vim.lsp.config(name, config)
end

vim.api.nvim_create_autocmd('LspAttach', {
	callback = function()
		vim.lsp.inlay_hint.enable()
		-- This will only work once you change file.
	end,
})

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

-- vim.api.nvim_create_autocmd('BufWritePre', {
--   pattern = '*.py',
--   callback = function()
--     local buf = vim.api.nvim_get_current_buf()
--     local filepath = vim.api.nvim_buf_get_name(buf)
--     local project_root = vim.fn.getcwd()
--
-- Ensure buffer is saved
--     vim.cmd('write')
--
-- Execute formatters explicitly from project root
--     vim.fn.system({ 'ruff', 'check', '--fix', filepath }, project_root)
--     vim.fn.system({ 'black', filepath }, project_root)
--     vim.fn.system({ 'isort', filepath }, project_root)
--
-- Reload the buffer quietly
--     vim.cmd('silent edit!')
--   end,
-- })

return python
