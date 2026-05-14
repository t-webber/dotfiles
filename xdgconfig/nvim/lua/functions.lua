------------------------
--- Global functions ---
------------------------

function TSUninstallAll()
        local ts = require('nvim-treesitter')
        for _, p in ipairs(ts.get_installed('parsers')) do
                ts.uninstall(p)
        end
end

function TSStart()
        vim.treesitter.start()
        -- vim.wo.foldexpr = 'v:lua.vim.treesitter.foldexpr()'
        vim.wo.foldexpr = 'v:lua.CustomFoldExpr()'
        vim.wo.foldmethod = 'expr'
end

function D(x) print(vim.inspect(x)) end
function R(x) require(x) end

--------------
--- Notify ---
--------------

local v = require('globals')
local statusline_notif = v.statusline_notif

vim.g[statusline_notif] = ''
local core_notify = vim.notify
vim.notify = function(msg, ...)
        if type(msg) ~= 'string' then error('found non string notification') end
        local key = msg:match('^You pressed the (.+) key too soon!')
        if key then
                vim.g[statusline_notif] = 'No ' .. key
                vim.cmd('redrawstatus')
                return
        end
        key = msg:match('The (.+) key is disabled!')
        if key then
                if key == '<Up>' then
                        key = ''
                elseif key == '<Left>' then
                        key = ''
                elseif key == '<Right>' then
                        key = ''
                elseif key == '<Down>' then
                        key = ''
                end
                vim.g[statusline_notif] = 'No ' .. key .. ' '
                vim.cmd('redrawstatus')
                return
        end
        local first, second = msg:match('Use (.+) instead of (.+)')
        if first and second then
                vim.g[statusline_notif] = first .. '  ' .. second
                vim.cmd('redrawstatus')
                return
        end
        if msg == 'No code actions available' then
                vim.g[statusline_notif] = msg
                vim.cmd('redrawstatus')
                vim.defer_fn(function()
                        if vim.g[statusline_notif] == msg then vim.g[statusline_notif] = '' end
                end, 3000)
                return
        end
        core_notify('core notif: ' .. msg, ...)
end
