local data = os.getenv('DATA')

local lazypath = data .. '/lazy/lazy.nvim'

if not (vim.uv or vim.loop).fs_stat(lazypath) then
	local lazyrepo = 'https://github.com/folke/lazy.nvim.git'
	local gitclone = {
		'git',
		'clone',
		'--filter=blob:none',
		'--branch=stable',
		lazyrepo,
		lazypath,
	}
	local out = vim.fn.system(gitclone)
	if vim.v.shell_error ~= 0 then
		vim.api.nvim_echo({
			{ 'Failed to clone lazy.nvim:\n', 'ErrorMsg' },
			{ out, 'WarningMsg' },
			{ '\nPress any key to exit...' },
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
		'lewis6991/gitsigns.nvim',
		config = function()
			require('gitsigns').setup({
				signs = {
					add = { text = '+' },
					change = { text = '~' },
					delete = { text = '_' },
					topdelete = { text = '‾' },
					changedelete = { text = '~~' },
				},
			})
		end,
		event = 'VeryLazy',
	},
	{
		'NeogitOrg/neogit',
		dependencies = {
			'nvim-lua/plenary.nvim',
			'sindrets/diffview.nvim',
		},
		config = function()
			require('neogit').setup({
				graph_style = 'ascii',
				status = { recent_commit_count = 100 },
				integrations = {
					diffview = true,
					disable_relative_line_numbers = false,
				},
			})
		end,
		cmd = 'Neogit',
	},
	{
		'topaxi/pipeline.nvim',
		build = 'make',
		cmd = 'Pipeline',
	},
	-----------
	--- Oil ---
	-----------
	{
		'stevearc/oil.nvim',
		dependencies = { 'nvim-tree/nvim-web-devicons' },
		config = function()
			require('oil').setup({
				columns = { 'icon' },
				delete_to_trash = true,
				win_options = {
					signcolumn = 'auto:2',
				},
				view_options = {
					show_hidden = true,
					is_always_hidden = function(name) return (name == '..') end,
				},
			})
		end,
	},
	{
		'refractalize/oil-git-status.nvim',
		dependencies = {
			'stevearc/oil.nvim',
		},
		config = true,
		event = 'VeryLazy',
	},
	------------
	--- Find ---
	------------
	{
		'folke/trouble.nvim',
		dependencies = { 'nvim-tree/nvim-web-devicons' },
		config = true,
		cmd = 'Trouble',
	},
	{
		'folke/todo-comments.nvim',
		dependencies = { 'nvim-lua/plenary.nvim' },
		event = 'VeryLazy',
		opts = {
			signs = true, -- show icons in the signs column
			sign_priority = 8, -- sign priority
			-- keywords recognized as todo comments
			keywords = {
				FIX = {
					icon = ' ',
					color = 'fix',
					alt = { 'BUG' },
					-- signs = false, -- configure signs for some keywords individually
				},
				TODO = { icon = ' ', color = 'todo' },
				HACK = { icon = ' ', color = 'hack' },
				WARN = {
					icon = ' ',
					color = 'warn',
				},
				PERF = {
					icon = ' ',
					color = 'perf',
				},
				NOTE = { icon = ' ', color = 'note', alt = { 'INFO' } },
				TEST = {
					icon = '⏲ ',
					color = 'test',
					alt = { 'TESTING', 'PASSED', 'FAILED' },
				},
			},
			gui_style = {
				fg = 'NONE', -- The gui style to use for the fg highlight group.
				bg = 'BOLD', -- The gui style to use for the bg highlight group.
			},
			merge_keywords = true, -- when true, custom keywords will be merged with the defaults
			-- highlighting of the line containing the todo comment
			-- * before: highlights before the keyword (typically comment characters)
			-- * keyword: highlights of the keyword
			-- * after: highlights after the keyword (todo text)
			highlight = {
				multiline = true,
				multiline_pattern = '^.', -- lua pattern to match the next multiline from the start of the matched keyword
				multiline_context = 10, -- extra lines that will be re-evaluated when changing a line
				before = '', -- "fg" or "bg" or empty
				keyword = 'wide', -- "fg", "bg", "wide", "wide_bg", "wide_fg" or empty. (wide and wide_bg is the same as bg, but will also highlight surrounding characters, wide_fg acts accordingly but with fg)
				after = 'fg', -- "fg" or "bg" or empty
				pattern = [[.*<(KEYWORDS)\s*:]], -- pattern or table of patterns, used for highlighting (vim regex)
				comments_only = true, -- uses treesitter to match keywords in comments only
				exclude = {}, -- list of file types to exclude highlighting
			},
			colors = {
				todo = { 'Todo' },
				hack = { 'Hack', '#00ff00' },
				warn = { 'Warn' },
				perf = { 'Perf' },
				note = { 'Note' },
				test = { 'Test' },
				fix = { 'Fix' },
			},
			search = {
				command = 'rg',
				args = {
					'--color=never',
					'--no-heading',
					'--with-filename',
					'--line-number',
					'--column',
				},
				-- regex that will be used to match keywords.
				-- don't replace the (KEYWORDS) placeholder
				pattern = [[\b(KEYWORDS):]], -- ripgrep regex
				-- pattern = [[\b(KEYWORDS)\b]], -- match without the extra colon. You'll likely get false positives
			},
		},
	},
	{
		'nvim-pack/nvim-spectre',
		dependencies = { 'nvim-lua/plenary.nvim' },
		config = function()
			require('spectre').setup({
				default = {
					find = {
						cmd = 'rg',
						options = {
							' --multiline',
						},
					},
					find_cmd = {
						'rg',
						'--multiline',
						'--no-heading',
						'--color=never',
						'--smart-case',
					},
				},
			})
		end,
		cmd = 'Spectre',
	},
	{
		't-webber/telescope.nvim',
		dependencies = {
			'nvim-lua/plenary.nvim',
			'nvim-telescope/telescope-frecency.nvim',
			'nvim-telescope/telescope-github.nvim',
			{ 'nvim-telescope/telescope-fzf-native.nvim', build = 'make' },
		},
		cmd = 'Telescope',
		config = function()
			local rg = { 'rg', '--files', '--sortr=modified' }
			require('telescope').setup({
				defaults = {
					border = false,
					winblend = 30,
					padding = 0,
					results_title = false,
					prompt_title = false,
					prompt_prefix = '🔍',
					preview_title = false,
					hidden = true,
					file_ignore_patterns = {},
					layout_strategy = 'horizontal',
					layout_config = {
						width = { padding = 0 },
						height = { padding = 0 },
					},
					on_open = function() vim.o.laststatus = 0 end,
					on_close = function() vim.o.laststatus = 2 end,
				},
				picker = {
					find_files = {
						find_command = rg,
					},
				},
				extensions = { fzf = {} },
			})
			require('telescope').load_extension('frecency')
			require('telescope').load_extension('gh')
			require('telescope').load_extension('fzf')
		end,
	},
	------------------------
	--- Language support ---
	------------------------
	{
		'ms-jpq/coq_nvim',
		dependencies = { 'ms-jpq/coq.artifacts', 'ms-jpq/coq.thirdparty' },
		config = function() vim.cmd('COQnow --shut-up') end,
		event = 'VeryLazy',
	},
	{ 's3rvac/vim-syntax-jira', event = 'VeryLazy' },
	{
		'stevearc/conform.nvim',
		config = function()
			local fmt = {}
			if os.getenv('DEVICE') ~= 'v' then fmt = require('fmt') end
			require('conform').setup({
				format_on_save = {
					timeout_ms = 1000,
				},
				formatters_by_ft = fmt,
			})
			require('conform').formatters.shfmt =
				{ append_args = { '-ln', 'bash' } }
		end,
		event = 'VeryLazy',
	},
	------------------
	--- Treesitter ---
	------------------
	{
		'nvim-treesitter/nvim-treesitter',
		build = ':TSUpdate',
		dependencies = { 'nvim-treesitter/nvim-treesitter-textobjects' },
		config = function()
			local v = require('tsletters')
			require('nvim-treesitter.configs').setup({
				ensure_installed = { 'c', 'lua', 'vim', 'vimdoc', 'query' },
				auto_install = true,
				highlight = {
					enable = true,
				},
				textobjects = {
					select = {
						enable = true,
						lookahead = true,
						keymaps = v.select,
					},
					move = {
						enable = true,
						set_jumps = true,
						goto_next_start = v.goto_next_start,
						goto_previous_start = v.goto_prev_start,
					},
				},
			})
			vim.treesitter.language.register('python', 'sage')
			vim.treesitter.language.register('ruby', 'conf')
		end,
		event = 'VeryLazy',
	},
	--------------
	--- Editor ---
	--------------
	{ 'kylechui/nvim-surround', config = true, event = 'VeryLazy' },
	{
		'folke/flash.nvim',
		event = 'VeryLazy', -- doesn't load on 'f'
		---@type Flash.Config
		opts = {
			modes = {
				char = {
					enabled = true,
					autohide = true,
					jump_labels = true,
					jump = {
						autojump = true,
					},
				},
			},
		},
		keys = {
			{
				's',
				mode = { 'n', 'x', 'o' },
				function() require('flash').jump() end,
				desc = 'Flash',
			},
			{
				'S',
				mode = { 'n', 'x', 'o' },
				function() require('flash').treesitter() end,
				desc = 'Flash Treesitter',
			},
			{
				'r',
				mode = 'o',
				function() require('flash').remote() end,
				desc = 'Remote Flash',
			},
			{
				'R',
				mode = { 'o', 'x' },
				function() require('flash').treesitter_search() end,
				desc = 'Treesitter Search',
			},
			{
				'<c-s>',
				mode = { 'c' },
				function() require('flash').toggle() end,
				desc = 'Toggle Flash Search',
			},
		},
	},
	{
		'chenasraf/text-transform.nvim',
		config = true,
		cmd = {
			'TtCamel',
			'TtPascal',
			'TtKebab',
			'TtSnake',
			'TtConst',
			'TtDot',
			'TtTitle',
		},
	},
	{
		'tpope/vim-commentary',
		event = 'VeryLazy',
	},
	{
		'karb94/neoscroll.nvim',
		config = true,
		event = 'VeryLazy',
	},
	{ 'm4xshen/hardtime.nvim', config = true, event = 'VeryLazy' },
	{
		'AckslD/nvim-neoclip.lua',
		dependencies = {
			{ 'kkharji/sqlite.lua', module = 'sqlite' },
			{ 'nvim-telescope/telescope.nvim' },
		},
		config = function() require('neoclip').setup() end,
		event = 'VeryLazy',
	},
	{
		'tversteeg/registers.nvim',
		dependencies = { 'MunifTanjim/nui.nvim' },
		config = function() require('registers').setup() end,
		event = 'VeryLazy',
	},
	----------------
	--- Terminal ---
	----------------
	{
		'chomosuke/term-edit.nvim',
		event = 'TermOpen',
		config = function() require('term-edit').setup({ prompt_end = '%$ ' }) end,
	},
}

-- Don't call the file lazy.lua otherwise... conflict!
require('lazy').setup({
	spec = spec,
	rocks = { enabled = false },
	ui = { silent = true, notify = false },
})
