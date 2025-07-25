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
		settings = {
			validate = "on",
		},
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
				-- 				cargo = { allFeatures = true },
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
	Python = {
		cmd = { "pyright-langserver", "--stdio" },
		filetypes = { "python" },
		settings = {
			python = {
				analysis = {
					-- 					typeCheckingMode = "strict",
					autoSearchPaths = true,
					useLibraryCodeForTypes = true,
					-- 					diagnosticSeverityOverrides = {
					-- 						reportMissingTypeStubs = "error",
					-- 					},
				},
			},
		},
	},
	Prisma = {
		cmd = { "prisma-language-server" },
		filetypes = { "prisma" },
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
