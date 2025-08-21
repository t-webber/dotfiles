local data = os.getenv("DATA")

local lazypath = data .. "/lazy/lazy.nvim"

if not (vim.uv or vim.loop).fs_stat(lazypath) then
	local lazyrepo = "https://github.com/folke/lazy.nvim.git"
	local gitclone = {
		"git",
		"clone",
		"--filter=blob:none",
		"--branch=stable",
		lazyrepo,
		lazypath,
	}
	local out = vim.fn.system(gitclone)
	if vim.v.shell_error ~= 0 then
		vim.api.nvim_echo({
			{ "Failed to clone lazy.nvim:\n", "ErrorMsg" },
			{ out, "WarningMsg" },
			{ "\nPress any key to exit..." },
		}, true, {})
		vim.fn.getchar()
		os.exit(1)
	end
end

-- Add lazypath to nvim runtime path. Needed to call require("lazy") and it's where the plugins are installed.
vim.opt.runtimepath:prepend(lazypath)

local spec = {
	-----------
	--- Git ---
	-----------
	{
		"lewis6991/gitsigns.nvim",
		config = true,
		event = "VeryLazy",
	},
	{
		"NeogitOrg/neogit",
		dependencies = {
			"nvim-lua/plenary.nvim",
			"sindrets/diffview.nvim",
		},
		config = function()
			require("neogit").setup({
				graph_style = "ascii",
				status = { recent_commit_count = 100 },
				integrations = {
					diffview = true,
					disable_relative_line_numbers = false,
				},
			})
		end,
		cmd = "Neogit",
	},
	---------------------
	--- File explorer ---
	---------------------
	{
		"nvim-tree/nvim-web-devicons",
		config = function()
			require("nvim-web-devicons").setup({ default = true })
		end,
	},
	{
		"stevearc/oil.nvim",
		dependencies = { "nvim-tree/nvim-web-devicons" },
		config = function()
			require("oil").setup({
				columns = { "icon" },
				delete_to_trash = true,
				win_options = {
					signcolumn = "auto:2",
				},
				view_options = {
					show_hidden = true,
					is_always_hidden = function(name)
						return (name == "..")
					end,
				},
			})
		end,
	},
	{
		"refractalize/oil-git-status.nvim",
		dependencies = {
			"stevearc/oil.nvim",
		},
		config = true,
		event = "VeryLazy",
	},
	---------------
	--- Finders ---
	---------------
	{
		"folke/trouble.nvim",
		dependencies = { "nvim-tree/nvim-web-devicons" },
		config = true,
		cmd = "Trouble",
	},
	{
		"nvim-pack/nvim-spectre",
		dependencies = { "nvim-lua/plenary.nvim" },
		config = function()
			require("spectre").setup({
				default = {
					find = {
						cmd = "rg",
						options = {
							" --multiline",
						},
					},
					find_cmd = {
						"rg",
						"--multiline",
						"--no-heading",
						"--color=never",
						"--smart-case",
					},
				},
			})
		end,
		cmd = "Spectre",
	},
	------------------------
	--- Language support ---
	------------------------
	{
		"ms-jpq/coq_nvim",
		dependencies = { "ms-jpq/coq.artifacts", "ms-jpq/coq.thirdparty" },
		config = function()
			vim.cmd("COQnow --shut-up")
		end,
		event = "VeryLazy",
	},
	{
		"nvim-treesitter/nvim-treesitter",
		build = ":TSUpdate",
		config = function()
			require("nvim-treesitter.configs").setup({
				auto_install = true,
				highlight = {
					enable = true,
				},
			})

			vim.treesitter.language.register("python", "sage")
			vim.treesitter.language.register("ruby", "conf")
		end,
	},
	{ "s3rvac/vim-syntax-jira", event = "VeryLazy" },
	--------------
	--- Editor ---
	--------------
	{ "kylechui/nvim-surround", config = true, event = "VeryLazy" },
	{
		"ggandor/leap.nvim",
		keys = { { "s", mode = "n" } },
		config = function()
			require("leap").set_default_mappings()
		end,
	},
	{
		"chenasraf/text-transform.nvim",
		config = true,
		cmd = {
			"TtCamel",
			"TtPascal",
			"TtKebab",
			"TtSnake",
			"TtConst",
			"TtDot",
			"TtTitle",
		},
	},
	-- 	{
	-- 		"tpope/vim-commentary",
	-- 		config = function()
	-- 			require("vim-commentary")
	-- 		end,
	-- 	},
	{
		"karb94/neoscroll.nvim",
		config = true,
		event = "VeryLazy",
	},
	{ "m4xshen/hardtime.nvim", config = true, event = "VeryLazy" },
	{
		"tversteeg/registers.nvim",
		dependencies = { "MunifTanjim/nui.nvim" },
		config = function()
			require("registers").setup()
		end,
		event = "VeryLazy",
	},
	----------------
	--- Terminal ---
	----------------
	{
		"chomosuke/term-edit.nvim",
		event = "TermOpen",
		config = function()
			require("term-edit").setup({ prompt_end = "%$ " })
		end,
	},
	-------------------
	-- Other plugins --
	-------------------
	{ import = "plugins" },
}
-- Don't call the file lazy.lua otherwise... conflict!
require("lazy").setup({
	spec = spec,
	rocks = { enabled = false },
	ui = { silent = true, notify = false },
})
