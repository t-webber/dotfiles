local formatters_by_ft = {

	-----------------
	--- Dedicated ---
	-----------------

	lua = { "stylua" },
	rust = { "rustfmt" },
	sql = { "sql_formatter" },
	python = { "ruff" },
	toml = { "taplo" },
	go = { "gofmt" },

	-------------
	--- Shell ---
	-------------

	sh = { "shfmt", ln = "bash" },
	bash = { "shfmt", ln = "bash" },

	----------------
	--- Prettier ---
	----------------

	javascript = { "prettier" },
	javascriptreact = { "prettier" },
	typescript = { "prettier" },
	typescriptreact = { "prettier" },
	html = { "prettier" },
	css = { "prettier" },
	json = { "prettier" },
	jsonc = { "prettier" },
	markdown = { "prettier" },

	--------------------
	--- Clang Format ---
	--------------------

	-- for .h: vim.g.c_syntax_for_h = 1 (see settings.lua)
	c = { "clang-format" },
	cpp = { "clang-format" },
}

return {
	"stevearc/conform.nvim",
	config = function()
		require("conform").setup({
			format_on_save = {
				timeout_ms = 1000,
			},
			formatters_by_ft = formatters_by_ft,
		})

		require("conform").formatters.shfmt =
			{ append_args = { "-ln", "bash" } }
	end,
}
