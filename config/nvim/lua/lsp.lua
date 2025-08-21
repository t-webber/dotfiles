local lsps = {
	Latex = {
		cmd = { "texlab" },
		filetypes = { "tex" },
	},
	Typescript = {
		cmd = { "typescript-language-server", "--stdio" },
		filetypes = {
			"typescript",
			"typescriptreact",
			"javascript",
			"javascriptreact",
		},
		root_dir = vim.fs.dirname(
			vim.fs.find({ "package.json" }, { upward = true })[1]
		),
	},
	Lua = {
		cmd = { "lua-language-server" },
		filetypes = { "lua" },
		settings = {
			Lua = {
				runtime = { version = "LuaJIT" },
				diagnostics = { globals = { "vim" } },
			},
		},
	},
	Rust = {
		cmd = { "rust-analyzer" },
		filetypes = { "rust" },
		settings = {
			["rust-analyzer"] = {
				--                 cargo = { allFeatures = true },
				check = { command = "clippy" },
			},
		},
	},
	Clang = {
		cmd = { "clangd" },
		filetypes = { "c", "cpp", "h", "hpp" },
	},
	Bash = {
		cmd = { "bash-language-server" },
		filetypes = { "sh", "bash" },
	},
	Go = {
		cmd = { "gopls" },
		filetypes = { "go" },
	},
	Python = {
		cmd = { "pyright-langserver", "--stdio" },
		filetypes = { "python" },
		settings = {
			python = {
				pythonPath = "/arm/projectscratch/pd/pj1100048/oyl_venvs/tomweb01/1cfb64a465746095d450d9582a3bfeee2cf5226e1737afe4c888e382fe9c9892/x86_64_rhel8_python3.12.7/bin/python",
				analysis = {
					typeCheckingMode = "strict",
					autoSearchPaths = true,
					useLibraryCodeForTypes = true,
					diagnosticSeverityOverrides = {
						reportUnknownVariableType = false,
						reportUnknownParameterType = false,
						reportMissingTypeStubs = false,
					},
				},
			},
		},
	},
}

for name, config in pairs(lsps) do
	vim.lsp.enable(name)
	vim.lsp.config(name, config)
end

vim.api.nvim_create_autocmd("LspAttach", {
	callback = function()
		vim.lsp.inlay_hint.enable()
		-- This will only work once you change file.
	end,
})

-- vim.api.nvim_create_autocmd('BufWritePre', {
-- pattern = '*.py',
-- callback = function()
-- local buf = vim.api.nvim_get_current_buf()
-- local filepath = vim.api.nvim_buf_get_name(buf)
--
-- vim.cmd('write')
--
-- vim.fn.system({ 'ruff', 'check', '--fix', filepath })
-- vim.fn.system({ 'black', filepath })
-- vim.fn.system({ 'isort', filepath })
--
-- vim.cmd('edit!')
-- end,
-- })

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
--
