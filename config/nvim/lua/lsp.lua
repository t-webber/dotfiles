local lsps = {
	Typescript = {
		cmd = { "typescript-language-server", "--stdio" },
		filetypes = { "typescript", "typescriptreact", "javascript", "javascriptreact" },
		root_dir = vim.fs.dirname(vim.fs.find({ "package.json" }, { upward = true })[1]),
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
	Rust = { cmd = { "rust-analyzer" }, filetypes = { "rust" } },
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
