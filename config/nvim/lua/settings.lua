-- show in-line errors
vim.diagnostic.config({ virtual_text = true })

local options = {
	relativenumber = true,
	number = true,
	cursorline = true,
	cmdheight = 0,
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
	tabline = "",
	-- 	winbar = "%!v:lua.___custom_statusline_content()",
	showtabline = 0,
	expandtab = true,
	-- 	spell = true,
	-- 	spelllang = { "en_gb" },
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
