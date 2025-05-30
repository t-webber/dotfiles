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
-- Don't call the file lazy.lua otherwise... conflict!
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
		config = function()
			require("neogit").setup({
				integrations = { diffview = true },
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

require("lazy").setup({
	spec = spec,
	rocks = { enabled = false },
	ui = { silent = true, notify = false },
})
