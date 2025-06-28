local borderchars = {
	prompt = {
		"─",
		"│",
		" ",
		"│",
		"┌",
		"┐",
		"│",
		"│",
	},
	results = {
		"─",
		"│",
		"─",
		"│",
		"├",
		"┤",
		"┘",
		"└",
	},
	preview = {
		"─",
		"│",
		"─",
		"│",
		"┌",
		"┐",
		"┘",
		"└",
	},
}

return {
	"t-webber/telescope.nvim",
	dependencies = { "nvim-lua/plenary.nvim" },
	cmd = "Telescope",
	config = function()
		local rg = { "rg", "--files", "--sortr=modified", "--hidden" }
		require("telescope").setup({
			defaults = {
				border = true,
				borderchars = borderchars,
				padding = 0,
				results_title = false,
				prompt_title = false,
				preview_title = false,
			},
			picker = {
				find_files = {
					hidden = true,
					find_command = rg,
				},
			},
		})
	end,
}
