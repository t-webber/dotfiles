local lazypath = os.getenv("DATA") .. "/lazy/lazy.nvim"

if not (vim.uv or vim.loop).fs_stat(lazypath) then
	local lazyrepo = "https://github.com/folke/lazy.nvim.git"
	local gitclone = { "git", "clone", "--filter=blob:none", "--branch=stable", lazyrepo, lazypath }
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

-- Add lazypath to nvim runtime path. Needed to call require("lazy")
vim.opt.runtimepath:prepend(lazypath)

local spec = {
	-----------
	--- Git ---
	-----------
	{
		"lewis6991/gitsigns.nvim",
		config = true,
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
	},
	---------------------
	--- File explorer ---
	---------------------
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
	},
	---------------
	--- Finders ---
	---------------
	{
		"t-webber/telescope.nvim",
		dependencies = { "nvim-lua/plenary.nvim" },
		config = function()
			local rg = { "rg", "--files", "--sortr=modified", "--hidden" }
			require("telescope").setup({
				defaults = {
					border = true,
					borderchars = {
						prompt = { "─", "│", " ", "│", "┌", "┐", "│", "│" },
						results = { "─", "│", "─", "│", "├", "┤", "┘", "└" },
						preview = { "─", "│", "─", "│", "┌", "┐", "┘", "└" },
					},
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
	},
	{
		"folke/which-key.nvim",
		event = "VeryLazy",
		opts = {
			-- your configuration comes here
			-- or leave it empty to use the default settings
			-- refer to the configuration section below
		},
		keys = {
			{
				",?",
				function()
					require("which-key").show({})
				end,
			},
		},
	},
	------------------------
	--- Language support ---
	------------------------
	{
		"ms-jpq/coq_nvim",
		config = function()
			vim.cmd("COQnow --shut-up")
		end,
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
		end,
	},
	{
		"Vigemus/iron.nvim",
		config = true,
	},
	--------------
	--- Editor ---
	--------------
	{ "kylechui/nvim-surround", config = true },
	{
		"ggandor/leap.nvim",
		dependencies = {
			"tpope/vim-repeat",
		},
		config = function()
			require("leap").set_default_mappings()
		end,
	},
	{
		"chenasraf/text-transform.nvim",
		config = true,
	},
	{
		"tris203/precognition.nvim",
		config = true,
	},
	{
		"karb94/neoscroll.nvim",
		config = true,
	},
	{ "m4xshen/hardtime.nvim", config = true },
	{
		"tversteeg/registers.nvim",
		dependencies = { "MunifTanjim/nui.nvim" },
		config = function()
			require("registers").setup({ window = { winblend = 0 } })
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
