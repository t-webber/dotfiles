-------------------------
--- Lualine functions ---
-------------------------

if vim.v.vim_did_enter == 1 then vim.api.nvim_clear_autocmds({}) end

local v = require('globals')
local statusline_notif = v.statusline_notif
local statusline_reg = v.statusline_reg

vim.api.nvim_create_autocmd({ 'RecordingEnter' }, {
        callback = function()
                local reg = vim.fn.reg_recording()
                if reg == '' then
                        vim.g[statusline_reg] = ''
                else
                        vim.g[statusline_reg] = reg
                end
        end,
})

vim.api.nvim_create_autocmd({ 'RecordingLeave' }, {
        callback = function() vim.g[statusline_reg] = '' end,
})

-----------------------
--- Other functions ---
-----------------------

local better_prisma_format = os.getenv('CMD') .. '/sh/paff'

vim.api.nvim_create_autocmd('BufWritePost', {
        pattern = '*.prisma',
        callback = function()
                local start = vim.loop.hrtime()

                os.execute(better_prisma_format)

                local paff = vim.loop.hrtime()

                local path = vim.api.nvim_buf_get_name(0)
                vim.cmd('bdelete!')
                vim.cmd('edit ' .. vim.fn.fnameescape(path))
                vim.cmd('filetype detect')

                local crazy_stuff = vim.loop.hrtime()

                local paff_time = math.floor((paff - start) / 1e6)
                local crazy_stuff_time = math.floor((crazy_stuff - paff) / 1e6)
                vim.g[statusline_notif] = paff_time .. ' & ' .. crazy_stuff_time
        end,
})

vim.api.nvim_create_autocmd('TextYankPost', {
        callback = function() vim.hl.on_yank() end,
})

vim.api.nvim_create_user_command('GitBlameLine', function()
        local line_number = vim.fn.line('.') -- Get the current line number. See `:h line()`
        local filename = vim.api.nvim_buf_get_name(0)
        print(vim.fn.system({ 'git', 'blame', '-L', line_number .. ',+1', filename }))
end, {})

vim.api.nvim_create_autocmd({ 'BufLeave', 'BufWinLeave' }, {
        callback = function(event)
                if vim.bo[event.buf].filetype == 'TelescopePrompt' then vim.o.laststatus = 2 end
        end,
})

-------------------
--- Tree sitter ---
-------------------

local available = {}
local ts = require('nvim-treesitter')
for _, i in ipairs({ 1, 2 }) do
        for _, p in ipairs(ts.get_available(i)) do
                available[p] = true
        end
end

local function get_installed()
        local installed = {}
        for _, p in ipairs(ts.get_installed('parsers')) do
                installed[p] = true
        end
        return installed
end

local installed = get_installed()

local function check_installed(p)
        local timer = vim.uv.new_timer()
        timer:start(
                100, -- first wait 100ms
                200, -- rerun every 200ms
                vim.schedule_wrap(function()
                        if get_installed()[p] then
                                timer:stop()
                                timer:close()
                                vim.schedule(TSStart) -- schedule to wait ts to parse the file
                        end
                end)
        )
end

vim.api.nvim_create_autocmd('FileType', {
        pattern = { '*' },
        callback = function(args)
                local p = args.match
                if p == 'sh' then p = 'bash' end
                if installed[p] == true then
                        TSStart()
                        return
                end
                if installed[p] == nil and available[p] == true then
                        ts.install(p)
                        check_installed(p)
                end
        end,
})

local spinner = { '⠋', '⠙', '⠹', '⠸', '⠼', '⠴', '⠦', '⠧', '⠇', '⠏' }

vim.api.nvim_create_autocmd('LspProgress', {
        callback = function(ev)
                local output
                if ev.data.params.value.kind == 'end' then
                        output = '󰄬'
                else
                        local time = vim.uv.hrtime()
                        local index = math.floor(time / 1e8) % #spinner + 1
                        local spin = spinner[index]
                        output = spin .. ' ' .. ev.data.params.value.title
                end
                vim.g[statusline_notif] = output
                vim.cmd('redrawstatus')
        end,
})
