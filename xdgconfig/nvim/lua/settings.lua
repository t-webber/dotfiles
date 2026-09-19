-- show in-line errors
vim.diagnostic.config({ virtual_text = true })

-- Sync clipboard between OS and Neovim.
-- Schedule the setting after `UiEnter` because it can increase startup-time.
-- Remove this option if you want your OS clipboard to remain independent.
-- See `:help 'clipboard'`
if os.getenv('DEVICE') ~= 'v' then vim.schedule(function() vim.o.clipboard = 'unnamedplus' end) end

vim.filetype.add({
        extension = {
                bib = 'bibtex',
        },
})

local options = {
        laststatus = 2,
        relativenumber = false,
        number = false,
        numberwidth = 1,
        cursorline = true,
        cmdheight = 0,
        mouse = '', -- disable mouse
        showmode = false, -- hide `--INSERT--` in command line
        ignorecase = true, -- ignore case in search...
        smartcase = true, -- ... except if \C or capitals present
        list = true, -- show Tab and trailing spaces
        swapfile = false,
        listchars = { tab = '  ' },
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
        foldopen = 'all',
        foldclose = 'all',
        -- 	foldlevel = 0,
        foldtext = 'v:lua.CustomFoldText()',
        fillchars = { eob = ' ' },
        -- 	foldnestmax = 1,
}

local globals = {
        c_syntax_for_h = 1, -- consider .h files as .c files
        mapleader = ',', -- leader key
        molten_image_provider = 'wezterm',
        molten_auto_open_output = false,
        molten_auto_image_popup = false,
        molten_virt_text_output = false,
        --         molten_output_win_max_height = 12,
        --         molten_virt_text_output = true,
        --         molten_wrap_output = true,
        loaded_node_provider = 0,
        loaded_perl_provider = 0,
        --         loaded_python3_provider = 1,
        --         python3_host_prog = vim.fn.getcwd() .. '/.venv/bin/python',
        loaded_ruby_provider = 0,
}

for k, v in pairs(options) do
        vim.opt[k] = v
end
for k, v in pairs(globals) do
        vim.g[k] = v
end

return options
