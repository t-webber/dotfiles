vim.opt.clipboard:append("unnamedplus")

-- show in-line errors
vim.diagnostic.config({ virtual_text = true })

local options = {
	relativenumber = true,
	number = true, -- absolute number for current line
	cursorline = true, -- needed to customise even only `cursorlinenr`
	cmdheight = 0, -- hide command line
	mouse = "", -- disable mouse
	showmode = false, -- hide `--INSERT--` in command line
	fillchars = { eob = " " },
	ignorecase = true, -- ignore case in search...
	smartcase = true, -- ... except if \C or capitals present
	list = true, -- show Tab and trailing spaces
	swapfile = false,
	termguicolors = true,
	completeopt = { "menuone", "noselect" },
	splitbelow = true, -- move cursor down on split
	splitright = true, -- move cursor to right on vsplit
	breakindent = true, -- indent after line wrap
	-- ,foldmethod = "expr" -- all functions are folded by default
	undofile = true,
	spell = true,
	spelllang = { "en_gb" },
}

local globals = {
	c_syntax_for_h = 1, -- consider .h files as .c files
	mapleader = ",", -- leader key
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
