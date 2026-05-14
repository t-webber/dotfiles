----------------
--- Doc vars ---
----------------

local keymap_display = {}
local keymap_taken = {}
local nb = 0

---------------
--- helpers ---
---------------

local function describe(keymap, action, description)
        local t = type(description)
        if t == 'string' then return description end
        if description then error('unsupported description ' .. description) end
        if type(action) == 'string' then
                return action:gsub('<CR>', '󰌑 '):gsub('<Esc>', '󱊷 ')
        end
        error('misssing description for ' .. keymap)
end

local function set_keymap_for_all_modes(modes, keymap, action, opts)
        for _, mode in ipairs(modes) do
                vim.keymap.set(mode, keymap, action, opts)
                nb = nb + 1
        end
end

local function add_keymap_to_doc(modes, keymap, action, description)
        local desc = describe(keymap, action, description)
        if keymap_taken[keymap] ~= nil then
                error(
                        'multiple keymaps for '
                                .. keymap
                                .. ' previous: '
                                .. keymap_taken[keymap]
                                .. ' new: '
                                .. desc
                )
        end
        keymap_taken[keymap] = desc
        table.insert(keymap_display, {
                keymap = keymap,
                description = desc,
                modes = modes,
        })
end

local default = { ignore = 666, opts = {} }

local function setk(modes, keymap, action, description, opts, ignore)
        opts = opts or default.opts
        opts['desc'] = description
        set_keymap_for_all_modes(modes, keymap, action, opts or default.opts)
        if ignore == nil then add_keymap_to_doc(modes, keymap, action, description) end
end

-------------
--- Modes ---
-------------

local n = { 'n' }
local ni = { 'n', 'i' }
local nv = { 'n', 'v' }
local nt = { 'n', 't' }
local niv = { 'n', 'i', 'v' }
local nit = { 'n', 'i', 't' }
local nvt = { 'n', 'v', 't' }
local nivt = { 'n', 'i', 'v', 't' }
local xo = { 'x', 'o' }
local nxo = { 'n', 'x', 'o' }

local i = { 'i' }
local t = { 't' }
local it = { 'i', 't' }

local troubleicon = '🚨 '
local fixicon = '🔧 '
local winicon = '🌄 '
local telescopeicon = '🔭 '
local giticon = '🦑 '
local oilicon = '🌻 '
local globalicon = '📢 '
local harpoonicon = '🔱 '
local lspicon = '💡 '
local tsoicon = '📦 '

---------------
--- General ---
---------------

setk(n, '<C-A>', 'v<C-a>')
setk(n, '<C-X>', 'v<C-x>')

for letter, scope in pairs({
        r = 'word',
        R = 'WORD',
}) do
        setk(nvt, ',' .. letter, function()
                local w = vim.fn.expand('<c' .. scope .. '>')
                local cmd = ':%s/\\<' .. w .. '\\>/'
                local keys = vim.api.nvim_replace_termcodes(cmd, true, false, true)
                vim.api.nvim_feedkeys(keys, 'c', false)
        end, '* for :%s on ' .. scope)
end

setk(n, ',t', ':te<CR>')
setk(n, ',fr', function() os.execute('setxkbmap fr') end, 'setxkbmap fr')
setk(n, ',o', function()
        if vim.fn.has('unix') == 1 then
                os.execute('xdg-open ' .. vim.fn.expand('<cWORD>'))
        else
                os.execute('open ' .. vim.fn.expand('<cWORD>'))
        end
end, 'open link')

setk(nv, 'ça', 'gg^vG$<CR>')
setk(nv, 'çy', 'gg^vG$y<CR>')
setk(nv, 'çs', '<Esc>:silent :w<CR>', 'save')
setk(niv, 'çx', '<Esc>:w<CR>:so<CR>', 'save and execute')
setk(niv, 'çd', function()
        vim.cmd('vnew | r # | normal! G<CR>')
        vim.cmd('diffthis')
        vim.cmd('wincmd h')
        vim.cmd('diffthis')
end, 'diff unsaved')

setk(nv, 'çe', function()
        vim.cmd('wincmd l')
        vim.cmd('q')
end, 'diff close')

setk(t, '<Esc>', [[<C-\><C-n>]])

setk(n, '<C-n>', ':enew | setlocal nonu nornu nomodifiable<CR>', 'Open empty immuable buffer')

setk(n, 'è ', ':noh<CR>')
setk(n, ',rx', function()
        vim.cmd('write')
        os.execute('pkill -USR1 -x sxhkd')
end, 'Reload sxhkd')

setk(n, ',i', ':NvimWebDeviconsHiTest<CR>')

-------------
--- MacOS ---
-------------

setk(nivt, '<F1>', '<C-o>')
setk(nivt, '<F2>', '<C-i>')
setk(nivt, '<A-(>', '{')
setk(nivt, '<A-)>', '}')
setk(nivt, '<D-:>', '\\')
setk(i, '<D-(>', '[')
setk(i, '<D-)>', ']')
setk(i, '<D-é>', '~')
setk(i, '<D-">', '#')
setk(i, '<D-_>', '─')

---------------
--- Harpoon ---
---------------

local function h() return require('harpoon') end

setk(n, '§a', function() h():list():add() end, harpoonicon .. 'add')
setk(n, '§s', function() h().ui:toggle_quick_menu(h():list()) end, harpoonicon .. 'list')

for __idx = 1, 10 do
        local __i = __idx
        if __i == 10 then __i = 0 end
        setk(
                n,
                '§' .. __i,
                function() h():list():select(1) end,
                harpoonicon .. 'select ' .. __idx .. 'th buffer'
        )
end

setk(n, '§h', function() h():list():prev() end, harpoonicon .. 'prev buffer')
setk(n, '§l', function() h():list():next() end, harpoonicon .. 'next buffer')

-----------------------------
--- Status line variables ---
-----------------------------

local function setglobs(letter, vars, name, action_letter, action_name, add)
        setk(n, 'ù' .. action_letter .. letter, function()
                for _, var in ipairs(vars) do
                        local value = add and var[2] or var[3]
                        if value == nil then value = '' end
                        if var[4] then
                                vim.o[var[1]] = value
                        else
                                local varname = require('globals')['statusline_' .. var[1]]
                                vim.g[varname] = value
                        end
                end
                vim.cmd.redrawstatus()
        end, globalicon .. action_name .. ' ' .. name)
end

local sl_fn = '%!v:lua.___custom_statusline_content()'

for letter, vars in pairs({
        -- map<k, vec<var>>:
        -- -- k = letter in ù<k>{y,d}
        -- -- var: [ <varname>, <value-on-y>, <value-on-d>, <is-opt=false>, <display>]
        -- for display, only first is read, except if name key exists
        a = { { 'laststatus', 2, 0, true, 'statusline' } },
        -- b tabs
        e = { { 'full_treesitter', 'active' } },
        d = { { 'filepath', 'dir' } },
        f = { { 'filepath', 'file' } },
        -- h hints
        i = { { 'time', 'active' } },
        l = { { 'linecol', 'active' } },
        n = {
                { 'relativenumber', true, false, true },
                { 'number', true, false, true },
        },
        m = { { 'notif', 'notif' } },
        p = { { 'filepath', 'path' } },
        s = { { 'statusline', sl_fn, nil, true, 'custom statusline' } },
        t = { { 'treesitter', 'active' }, { 'full_treesitter', '' } },
        w = { { 'notif', 'notif' }, { 'reg', 'reg' }, name = 'notif + reg' },
        y = { { 'battery', 'active' } },
}) do
        local name = vars.name or vars[1][1]
        setglobs(letter, vars, name, 'y', 'add', true)
        setglobs(letter, vars, name, 'd', 'del', false)
end

for k, v in pairs({
        ['@'] = {
                function(x) vim.g[require('globals').pyfmt] = x end,
                '',
                'active',
                'ruff',
                fixicon,
        },
        o = {
                function(x) require('conform').formatters_by_ft = x end,
                require('fmt'),
                {},
                'formatting',
                fixicon,
        },
        z = {
                function(x) vim.wo[0][0].foldmethod = x end,
                'expr',
                'manual',
                'automatic folding',
        },
        b = {
                function(x) vim.opt.listchars = x end,
                require('data').listchars,
                { space = ' ', tab = ' ' },
                'tabs and spaces',
        },
        ['$'] = {
                function(x) vim.cmd('Copilot ' .. x) end,
                'enable',
                'disable',
                'copilot',
                '',
        },
        h = {
                function(x) vim.lsp.inlay_hint.enable(x) end,
                true,
                false,
                'inlay hints',
                lspicon,
        },
}) do
        local ic = v[5] or globalicon
        setk(n, 'ùy' .. k, function() v[1](v[2]) end, ic .. 'enable ' .. v[4])
        setk(n, 'ùd' .. k, function() v[1](v[3]) end, ic .. 'disable ' .. v[4])
end

-------------------------
--- Window navigation ---
-------------------------

for letter, command in pairs({
        h = '<C-w>h',
        j = '<C-w>j',
        k = '<C-w>k',
        l = '<C-w>l',
        g = ':vert res +20<CR>',
        m = ':vert res -20<CR>',
        ['ù'] = ':vert res -1<CR>',
        f = ':vert res +1<CR>',
        u = ':res +20<CR>',
        n = ':res -20<CR>',
        b = ':res -1<CR>',
        y = ':res +1<CR>',
}) do
        local keymap = '<A-' .. letter .. '>'
        setk(nit, keymap, '<C-\\><C-n>' .. command)
        keymap = '<D-' .. letter .. '>'
        setk(n, keymap, command)
        setk(it, keymap, '<C-\\><C-n>' .. command, winicon .. command, default.opts, default.ignore)
end

setk(n, '<C-*>', ':vsplit | term<CR>')
setk(n, '<C-ù>', ':split | term<CR>')

-------------------------
--- Window reordering ---
-------------------------

for letter, settings in pairs({
        r = { 'vsplit' },
        R = { 'vsplit', true },
        t = { 'split' },
        T = { 'split', true },
}) do
        local command = settings[1]
        local reverse = settings[2]
        local keymaps = { '<A-' .. letter .. '>', '<C-w>' .. letter }

        for _, keymap in pairs(keymaps) do
                setk(nivt, keymap, function()
                        local file = vim.fn.expand('%:f')
                        vim.cmd('hide')
                        if reverse then
                                local cur = vim.api.nvim_get_current_win()
                                vim.cmd(command)
                                vim.api.nvim_set_current_win(cur)
                        else
                                vim.cmd(command)
                        end
                        vim.cmd('buf ' .. file)
                end, winicon .. 'switch to ' .. command .. (reverse and ' reverse' or ''))
        end
end

-----------------------
--- Window exchange ---
-----------------------

for letter, direction in pairs({
        w = 'h',
        x = 'j',
        c = 'k',
        v = 'l',
}) do
        setk(nivt, '<A-' .. letter .. '>', function()
                local old_file = vim.fn.expand('%:f')
                local old_win = vim.api.nvim_get_current_win()

                vim.cmd('wincmd ' .. direction)

                local new_file = vim.fn.expand('%:f')
                local new_win = vim.api.nvim_get_current_win()

                if old_win == new_win then return end

                vim.cmd('buf ' .. old_file)
                vim.cmd('wincmd p')
                vim.cmd('buf ' .. new_file)
                vim.cmd('wincmd ' .. direction)
        end, winicon .. 'switch to ' .. direction)
end

----------------
--- Markdown ---
----------------

local mdprefix = 'ç'
local md = {
        i = 'ysiw*',
        I = 'ds*',
        u = 'ysiw_',
        U = 'ds_',
}

for k, v in pairs(md) do
        table.insert(keymap_display, { keymap = mdprefix .. k, description = v, modes = { 'n' } })
end

setk(n, 'ç', function()
        local key = vim.fn.getcharstr()
        local cmd = md[key]
        if cmd then
                local termcode = vim.api.nvim_replace_termcodes(cmd, true, false, true)
                vim.api.nvim_feedkeys(termcode, 'm', false)
        else
                vim.api.nvim_feedkeys('ç' .. key, 'n', false)
        end
end, nil, default.opts, default.ignore)

-----------------
--- Iron REPL ---
-----------------

setk(n, 'è:', ':IronAttach<CR>')
setk(n, 'è!', ':IronHide<CR>')
setk(n, '<C-è>', function()
        local line = vim.api.nvim_get_current_line()
        print(line)
        if line == '' then line = ' ' end
        vim.api.nvim_command('IronSend ' .. line:gsub(' ', '\\ '))

        vim.api.nvim_command('normal! j')
end, 'IronSend')

-----------
--- Git ---
-----------

for letter, action in pairs({
        h = 'preview_hunk',
        i = 'preview_hunk_inline',
        t = 'toggle_current_line_blame',
        s = 'toggle_signs',
        l = 'blame_line',
        b = 'blame',
        d = 'reset_hunk',
}) do
        setk(n, 'g' .. letter, ':Gitsigns ' .. action .. '<CR>', giticon .. action)
end

setk(n, 'ga', ':te git add -p<CR>', giticon .. 'add -p')
setk(n, 'go', ':Neogit<CR>', giticon .. 'neogit')
setk(n, 'gn', ':Pipeline<CR>', giticon .. 'GitHub Actions')

-----------------
--- Telescope ---
-----------------

local function tpick(finder, previewer, sorter)
        return require('telescope.pickers')
                .new({}, { finder = finder, previewer = previewer, sorter = sorter })
                :find()
end

local function tconf() return require('telescope.config').values end
local function tfind() return require('telescope.finders') end
local function tb() return require('telescope.builtin') end
local function frec() return require('telescope').extensions.frecency.frecency end

for letter, cmd in pairs({
        f = { 'git_files', { show_untracked = true } },
        g = { 'git_files' },
        d = { 'find_files' },
        s = { 'git_status' },
        h = { 'highlights' },
        e = { 'live_grep' },
        r = { 'registers' },
        z = { 'frecency', { workspace = 'CWD' }, fn = frec },
        c = { 'command_history' },
        a = { 'search_history' },
        y = { 'lsp_dynamic_workspace_symbols' },
}) do
        setk(n, 'é' .. letter, function()
                vim.o.laststatus = 0
                local opts = { cwd = vim.fs.root(0, { '.git' }) }
                opts = vim.tbl_extend('force', opts, cmd[2] or {})
                if cmd['fn'] == nil then
                        tb()[cmd[1]](opts)
                else
                        cmd['fn']()
                end
                vim.o.laststatus = 2
        end, telescopeicon .. cmd[1])
end

local function settg(letter, cmd, description, functions)
        setk(n, 'é' .. letter, function()
                vim.o.laststatus = 0
                local finder = tfind().new_oneshot_job(
                        cmd,
                        { entry_maker = functions and functions.entry_maker }
                )
                tpick(finder, tconf().generic_sorter({}), functions and functions.attach_mappings)
                vim.o.laststatus = 2
        end, description)
end

settg('i', {
        'gh',
        'issue',
        'list',
        '--limit',
        '500',
}, telescopeicon .. 'issues')

settg('p', {
        'gh',
        'pr',
        'list',
        '--limit',
        '500',
}, telescopeicon .. 'prs')

settg('n', {
        'fd',
        '--no-ignore',
        '--hidden',
        '-E target',
        '-E git',
        '-E node_modules',
        '-E venv',
}, telescopeicon .. 'hidden files')

settg(
        'x',
        {
                'clipcatctl',
                'list',
        },
        telescopeicon .. 'clipcat',
        {
                entry_maker = function(entry)
                        local id, content = entry:match('^(.-):%s*(.*)$')
                        local display = string.gsub(content, '\\n', ' | ')
                        return { value = id, display = display, ordinal = content }
                end,
                attach_mappings = function(prompt_bufnr, map)
                        local actions = require('telescope.actions')
                        local action_state = require('telescope.actions.state')

                        local function print_selection()
                                local selected_entry = action_state.get_selected_entry()
                                local selected_entry_id = selected_entry.value

                                local clipboard_entry =
                                        vim.fn.system('clipcatctl get ' .. selected_entry_id)

                                local clipboard_entry_fixed = string.gsub(
                                        string.gsub(clipboard_entry, '\\t', '\t'),
                                        '\\n',
                                        '\n'
                                )
                                vim.fn.setreg('+', clipboard_entry_fixed)
                                actions.close(prompt_bufnr)
                        end

                        map('i', '<CR>', print_selection)
                        map('n', '<CR>', print_selection)

                        return true
                end,
        }
)

setk(n, 'ét', function(opts)
        local make_entry = require('telescope.make_entry')

        opts = opts or {}
        local cwd = opts.cwd or vim.uv.cwd()

        local finder = tfind().new_async_job({
                command_generator = function(prompt)
                        if not prompt or prompt == '' then return nil end

                        local pieces = vim.split(prompt, '  ')
                        local args = { 'rg' }
                        if pieces[1] then
                                table.insert(args, '-e')
                                table.insert(args, pieces[1])
                        end
                        if pieces[2] then
                                table.insert(args, '-g')
                                table.insert(args, pieces[2])
                        end

                        return vim.tbl_flatten({
                                args,
                                {
                                        '--color=never',
                                        '--no-heading',
                                        '--with-filename',
                                        '--line-number',
                                        '--column',
                                        '--smart-case',
                                },
                        })
                end,
                entry_maker = make_entry.gen_from_vimgrep(opts),
                cwd = cwd,
        })

        tpick(finder, tconf().grep_previewer(opts), require('telescope.sorters').empty())
end, 'find content in files')

setk(n, 'él', function()
        local file_paths = {}
        for _, item in ipairs(h():list().items) do
                table.insert(file_paths, item.value)
        end

        tpick(
                tfind().new_table({
                        results = file_paths,
                }),
                tconf().file_previewer({}),
                tconf().generic_sorter({})
        )
end, telescopeicon .. harpoonicon)

-----------
--- Oil ---
-----------

local icons = require('icons')

for _, folder in ipairs(icons) do
        setk(
                n,
                'è' .. folder.short,
                function() require('oil').open(folder.var or '!') end,
                oilicon .. folder.icon
        )
end

setk(n, 'èk', ':Oil<CR>', oilicon .. '.')
setk(n, 'èj', function()
        local path = vim.fn.expand('<cfile>')
        require('oil').open(path)
end, oilicon .. 'open file')

setk(n, 'èg', function() require('oil').open(vim.loop.cwd()) end, oilicon .. 'open cwd')

-----------
--- LSP ---
-----------

--> errors <--

setk(n, 'àn', '%!jq -S .')
setk(n, 'àb', function() vim.lsp.buf.hover() end, lspicon .. 'buf hover')
setk(n, 'àw', function() vim.diagnostic.open_float() end, lspicon .. 'open float')

setk(n, '<F12>', function() vim.lsp.buf.code_action() end, '💡 code action')
setk(n, 'àf', function() vim.lsp.buf.code_action() end, '💡 code action')

setk(n, 'àa', ':Trouble diagnostics<CR>', troubleicon .. 'diagnostics')
setk(
        n,
        'àv',
        ':Trouble diagnostics win.type=split win.position=right win.width=120<CR>',
        troubleicon .. 'diagnostics vert'
)

setk(
        n,
        'à)',
        function() require('trouble').next({ skip_groups = true, jump = true }) end,
        troubleicon .. 'next'
)

setk(
        n,
        'à(',
        function() require('trouble').prev({ skip_groups = true, jump = true }) end,
        troubleicon .. 'prev'
)

setk(n, 'àt', function()
        os.execute('cargo test -- --nocapture >output.txt 2>&1')
        vim.cmd('edit output.txt')
        vim.cmd('normal! G')
        vim.cmd([[syntax match CustomSuccess  /\<\(ok\|passed\)\>/]])
        vim.cmd([[syntax match CustomWarning  /\<\(ignored\|filtered out\)\>/]])
        vim.cmd([[syntax match CustomError /\<\(failed\|FAILED\|error\|failures\)\>/]])
end, '🧪 cargo test')

local function move_and_open(action)
        return function()
                vim.diagnostic.setqflist({ open = false })
                vim.api.nvim_command(action)
                vim.defer_fn(function() vim.diagnostic.open_float(nil, { focus = false }) end, 50)
        end
end

setk(n, 'àl', move_and_open('cnext'), 'cnext')
setk(n, 'àh', move_and_open('cprev'), 'cprev')
setk(n, 'àk', ':cclose<CR>', 'cclose')
setk(n, 'àj', ':copen<CR>', 'copen')

--> motion <--

setk(n, 'àd', function() vim.lsp.buf.definition({}) end, 'Go to definition')

--> lsp status <--

setk(n, 'às', ':checkhealth lsp<CR>')

setk(n, 'àp', function()
        local clients = vim.lsp.get_clients()
        local buf = vim.api.nvim_get_current_buf()
        for _, client in ipairs(clients) do
                if
                        vim.lsp.get_client_by_id(client.id)
                        and vim.lsp.buf_is_attached(buf, client.id)
                then
                        print('Active LSP client:', client.name)
                end
        end
end, 'Print active lsp clients')

------------
--- Case ---
------------

setk(nv, ',cc', ':TtCamel<CR>', '🐪 camel')
setk(nv, ',cp', ':TtPascal<CR>', '🐪 pascal')
setk(nv, ',ck', ':TtKebab<CR>', '🐪 kebab')
setk(nv, ',cs', ':TtSnake<CR>', '🐪 snake')
setk(nv, ',co', ':TtConst<CR>', '🐪 const')
setk(nv, ',cd', ':TtDot<CR>', '🐪 dot')
setk(nv, ',ct', ':TtTitle<CR>', '🐪 title')

-------------------------
--- Comment/Uncomment ---
-------------------------

setk(n, '<C-!>', function()
        local comment
        if vim.bo.filetype == 'python' then
                comment = '#'
        else
                local comments = vim.bo.comments
                comment = comments:match('([^:]+)$')
        end

        local pattern = '^%s*(' .. vim.pesc(comment) .. '+)%s*'

        local line = vim.api.nvim_get_current_line()

        local _, _, match = line:find(pattern)

        if match then
                line = line:gsub(pattern, '')
        else
                line = comment .. ' ' .. line
        end

        vim.api.nvim_set_current_line(line)
        vim.cmd('normal! j')
end, 'Comment/Uncomment line')

----------------------
--- Random strings ---
----------------------

setk(n, ',s', function()
        local charset = {}
        for c = 48, 57 do
                table.insert(charset, string.char(c))
        end -- 0-9
        for c = 65, 90 do
                table.insert(charset, string.char(c))
        end -- A-Z
        for c = 97, 122 do
                table.insert(charset, string.char(c))
        end -- a-z

        math.randomseed(os.time() + math.floor(math.random() * 10000))
        local s = {}
        for j = 1, 10 do
                s[j] = charset[math.random(#charset)]
        end
        vim.api.nvim_put({ table.concat(s) }, 'c', true, true)
end, 'Generate a random string')

------------
--- Slop ---
------------

vim.api.nvim_set_keymap(
        'i',
        '<C-CR>',
        'copilot#Accept("<CR>")',
        { silent = true, script = true, expr = true }
)

-------------------
--- Tree sitter ---
-------------------

local function rts(func)
        local scope, _ = func:match('^([^_]*)_')
        local function missingarg(arg)
                local function callback()
                        return require('nvim-treesitter-textobjects.' .. scope)[func](
                                arg,
                                'textobjects'
                        )
                end
                return callback
        end
        return missingarg
end

for _, k in ipairs({ '(', '[', ']', ')' }) do
        pcall(vim.keymap.del, 'n', k .. 'a')
end

for key, scope in pairs({
        q = '@assignment',
        t = '@attribute',
        b = '@block',
        k = '@call',
        c = '@class',
        h = '@comment',
        z = '@conditional',
        o = '@function',
        l = '@loop',
        n = '@number',
        e = '@parameter',
        x = '@regex',
        r = '@return',
        f = '@scopename',
        s = '@statement',
}) do
        for prefix, func in pairs({
                [''] = rts('select_textobject'),
                [')'] = rts('move_next_start'),
                [']'] = rts('move_next_end'),
                ['('] = rts('move_previous_start'),
                ['['] = rts('move_previous_end'),
                ['{'] = rts('move_next'),
                ['}'] = rts('move_previous'),
                ['>'] = rts('swap_next'),
                ['<'] = rts('swap_previous'),
        }) do
                local s = scope:sub(2)
                if vim.tbl_contains({ '<', '>' }, prefix) then
                        setk(n, prefix .. key, func(scope .. '.inner'), tsoicon .. s)
                        goto continue
                end

                local modes
                if prefix == '' then
                        modes = xo
                else
                        modes = nxo
                end
                for l, suffix in pairs({ i = '.inner', a = '.outer' }) do
                        local d = tsoicon .. s .. '.' .. l
                        setk(modes, prefix .. l .. key, func(scope .. suffix), d)
                end
                ::continue::
        end
end

---------------------
--- Custom helper ---
---------------------

local function fmt_keymap(keymap, inline)
        local modes
        if inline then
                modes = '  '
        else
                modes = '\t ' .. table.concat(keymap.modes) .. '\t'
        end
        local description = keymap.description or 'nil'
        local line = keymap.keymap:gsub(' ', '␣') .. modes .. description

        return line
end

local msg = require('globals').statusline_notif

local timer = vim.loop.new_timer()
timer:start(
        0,
        20 * 1000,
        vim.schedule_wrap(function()
                math.randomseed(os.time())
                local idx = math.random(0, #keymap_display - 1)
                local keymap = keymap_display[idx]
                if keymap == nil then
                        timer:stop()
                        timer:close()
                        vim.g[msg] = 'invalid idx ' .. idx .. ' for len ' .. #keymap_display
                else
                        vim.g[msg] = fmt_keymap(keymap, true)
                end
        end)
)

local function load_lines(width)
        local lines = {
                nb .. ' keymaps (q to exit)',
                string.rep('-', width),
                'z=\t n\t open spell suggestions',
                ']s\t n\t next spell error',
                '[s\t n\t previous spell error',
                'zg\t n\t mark word as correct',
                'za\t n\t toggle fold',
                'zo\t n\t open fold',
                'zc\t n\t close fold',
                'zR\t n\t unfold all',
                'zM\t n\t fold all',
                string.rep('-', width),
        }

        for _, keymap in ipairs(keymap_display) do
                table.insert(lines, fmt_keymap(keymap))
        end

        return lines
end

local function open_ui()
        local ui = vim.api.nvim_list_uis()[1]
        local width = 60

        local lines = load_lines(width)

        local height = #lines
        local col = math.floor((ui.width - width) / 2)
        local row = math.floor((ui.height - height) / 2)

        local buf = vim.api.nvim_create_buf(false, true)
        vim.api.nvim_buf_set_lines(buf, 0, -1, false, lines)

        local opts = {
                style = 'minimal',
                relative = 'editor',
                width = width,
                height = height,
                row = row,
                col = col,
                border = 'rounded',
        }

        local win = vim.api.nvim_open_win(buf, true, opts)
        vim.api.nvim_win_set_option(win, 'winhl', 'Normal:Normal')

        vim.api.nvim_buf_set_keymap(buf, 'n', 'q', ':close<CR>', { noremap = true, silent = true })
end

vim.api.nvim_set_keymap('n', 'g?', '<Nop>', {
        callback = open_ui,
})
