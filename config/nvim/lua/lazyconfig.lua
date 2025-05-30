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
	------------
	-- Window --
	------------
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
	{
		"nvim-telescope/telescope.nvim",
		dependencies = { "nvim-lua/plenary.nvim" },
		config = function()
			local rg = { "rg", "--files", "--sortr=modified", "--hidden" }
			require("telescope").setup({
				picker = {
					find_files = {
						hidden = true,
						find_command = rg,
					},
				},
			})
		end,
	},
	-------------------
	-- File explorer --
	-------------------
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
				keymaps = {
					["el"] = { "actions.select" },
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
	------------
	-- Editor --
	------------
	{
		"ms-jpq/coq_nvim",
		config = function()
			vim.cmd("COQnow")
		end,
	},
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
		"nvim-treesitter/nvim-treesitter",
		build = ":TSUpdate",
		config = function()
			require("nvim-treesitter.configs").setup({
				auto_install = true,
				highlight = {
					enable = true,
				},
			})
		end,
	},
	{
		"chenasraf/text-transform.nvim",
		config = true,
	},
	{
		"Vigemus/iron.nvim",
		config = true,
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
