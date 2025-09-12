-- show in-line errors
vim.diagnostic.config({ virtual_text = true })

-- Sync clipboard between OS and Neovim.
--  Schedule the setting after `UiEnter` because it can increase startup-time.
--  Remove this option if you want your OS clipboard to remain independent.
--  See `:help 'clipboard'`
if os.getenv('DEVICE') ~= 'v' then
	vim.schedule(function()
		vim.o.clipboard = 'unnamedplus'
	end)
end

local options = {
	relativenumber = true,
	number = true,
	numberwidth = 1,
	cursorline = true,
	cmdheight = 0,
	mouse = '', -- disable mouse
	showmode = false, -- hide `--INSERT--` in command line
	fillchars = { eob = ' ' },
	ignorecase = true, -- ignore case in search...
	smartcase = true, -- ... except if \C or capitals present
	list = true, -- show Tab and trailing spaces
	listchars = {
		tab = '▹ ', -- shows tabs as → followed by a space
		trail = '·', -- shows trailing spaces as ·
		nbsp = '␣', -- optional, for non-breaking spaces
	},
	swapfile = false,
	termguicolors = true,
	completeopt = { 'menuone', 'noselect' },
	splitbelow = true, -- move cursor down on split
	splitright = true, -- move cursor to right on vsplit
	breakindent = true, -- indent after line wrap
	-- ,foldmethod = "expr" -- all functions are folded by default
	undofile = true,
	tabline = '',
	-- 	winbar = "%!v:lua.___custom_statusline_content()",
	showtabline = 0,
	expandtab = true,
	-- 	spell = true,
	-- 	spelllang = { "en_gb" },
	winborder = 'single',
}

local globals = {
	c_syntax_for_h = 1, -- consider .h files as .c files
	mapleader = ',', -- leader key
	loaded_node_provider = 0,
	loaded_perl_provider = 0,
	loaded_python3_provider = 0,
	loaded_ruby_provider = 0,
}

for k, v in pairs(options) do
	vim.opt[k] = v
end
for k, v in pairs(globals) do
	vim.g[k] = v
end
