vim.filetype.add({
	extension = {
		sage = "python",
	},
})

local lsps = {
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
	Clang = {
		cmd = { "clangd" },
		filetypes = { "c", "cpp", "h", "hpp" },
	},
	Bash = {
		cmd = { "bash-language-server" },
		filetypes = { "sh", "bash" },
	},
	Python = {
		cmd = { "pyright-langserver", "--stdio" },
		filetypes = { "python" },
		settings = {
			python = {
				analysis = {
					typeCheckingMode = "strict",
					autoSearchPaths = true,
					useLibraryCodeForTypes = true,
					diagnosticSeverityOverrides = {
						reportMissingTypeStubs = "error",
					},
				},
			},
		},
	},

	-- 	{
	-- 		cmd = { "ruff", "server" },
	-- 		filetypes = { "py", "python", "sage" },
	-- 	},
}

for name, config in pairs(lsps) do
	vim.lsp.enable(name)
	vim.lsp.config(name, config)
end
