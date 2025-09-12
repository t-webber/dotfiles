local v = require('globals')
local statusline_notif = v.statusline_notif
local statusline_notif_enable = v.statusline_notif_enable
local statusline_reg = v.statusline_reg
local statusline_linecol = v.statusline_linecol
local statusline_filepath = v.statusline_filepath
local statusline_treesitter = v.statusline_treesitter
local statusline_full_treesitter = v.statusline_full_treesitter
local statusline_battery = v.statusline_battery
local statusline_time = v.statusline_time

-------------
--- Utils ---
-------------

local function get_abs_path()
        local winid = vim.g.statusline_winid
        local bufnr = vim.fn.winbufnr(winid)
        local bufname = vim.fn.bufname(bufnr)
        return vim.fn.fnamemodify(bufname, ':p')
end

local function make_hg(hg_name) return '%#' .. hg_name .. '#' end

local reset_colour = make_hg('CustomStatusLineDefault')

local mode_names = {
        n = 'Normal',
        i = 'Insert',
        v = 'Visual',
        V = 'VisualLine',
        ['\22'] = 'VisualBlock',
        ['\19'] = 'SelectBlock',
        s = 'Select',
        S = 'SelectLine',
        c = 'Command',
        R = 'Replace',
        t = 'Terminal',
}

local function get_mode_name()
        -- mode can be nil when transitioning (e.g. in Neogit)
        return mode_names[vim.fn.mode()] or 'Unknown'
end

local function reduce_path(path)
        local icon_paths = require('icons')
        for _, icon_path in ipairs(icon_paths) do
                local subpath = path:match((icon_path.var or '!!') .. '/(.*)')
                if subpath ~= nil then return icon_path.icon .. subpath end
        end
        return path
end

local function read_unchecked(filename)
        local file = io.open(filename, 'r')
        if not file then return '!!!!!!!!!!!!!' end
        local level = file:read('*l')
        file:close()
        return level
end

-- local colours = require('colours')
-- local fnction get_non_ts_hg(file)
-- 	local capitalised = string.upper(string.sub(file, 1, 1))
-- 		.. string.sub(file, 2)
-- 	local hg = 'DevIcon' .. capitalised
-- 	if colours[hg] ~= nil then
-- 		return make_hg(hg)
-- 	end
-- 	return '[' .. file .. ']'
-- end

----------------------------
--- Depend on global var ---
----------------------------

local function extract_dir_file(abs_path, dir)
        local parts = vim.split(abs_path, '/')
        local nb = #parts
        if nb <= 1 then
                return abs_path
        elseif dir then
                return parts[nb - 1] .. '/' .. parts[nb]
        else
                return parts[nb]
        end
end

local function get_displayed_file_name()
        local display = vim.g[statusline_filepath]
        local abs_path = get_abs_path()
        if display == 'path' then
                return abs_path
        elseif display == 'dir' then
                return extract_dir_file(abs_path, true)
        elseif display == 'file' then
                return extract_dir_file(abs_path, false)
        elseif display == '' then
                return ''
        else
                return '!!!' .. display .. '!!!'
        end
end

---------------------------
--- Statusline elements ---
---------------------------

local function sl_sep(char)
        if char == nil then char = '|' end
        local mode = get_mode_name()
        local sep = '' .. char .. ''
        return make_hg('CustomStatusLine' .. mode .. 'Symbol') .. sep .. reset_colour
end

local function sl_acer_battery()
        local level = read_unchecked('/sys/class/power_supply/BAT1/capacity')
        local status = read_unchecked('/sys/class/power_supply/BAT1/status')
        if tonumber(level) < 20 and status ~= 'Charging' and os.getenv('NO_DUNST') == nil then
                vim.system({
                        '/bin/notify-send',
                        '-u',
                        'critical',
                        'low battery (nvim) (' .. level .. '%)',
                })
        end
        if status == 'Full' then
                vim.system({
                        '/bin/notify-send',
                        '-u',
                        'critical',
                        'battery full (nvim)',
                })
        end
        local hg = make_hg('CustomStatusLineBattery' .. status)
        return sl_sep() .. hg .. string.format('%s', level)
end

local function sl_battery()
        if vim.g[statusline_battery] == '' then return '' end
        local dev = os.getenv('DEVICE')
        if dev == 'acer' then return sl_acer_battery() end
        --         if dev == 'mac' then return sl_mac_battery() end
        return ''
end

local function sl_position()
        if vim.g[statusline_linecol] == '' then return '' end
        local line = vim.fn.line('.')
        local col = vim.fn.col('.')
        return make_hg('CustomStatusLinePosition') .. string.format('%d:%d', line, col)
end

local function sl_messages()
        local inner_sep = ''
        local outer_sep = ''

        if vim.g[statusline_notif_enable] == '' then return '' end

        local has_reg = vim.g[statusline_reg] ~= ''
        local has_notif = vim.g[statusline_notif] ~= ''

        if has_reg and has_notif then inner_sep = sl_sep() end
        if has_reg or has_notif then outer_sep = sl_sep() end

        return outer_sep
                .. make_hg('CustomStatusLineHardtime')
                .. vim.g[statusline_notif]
                .. inner_sep
                .. make_hg('CustomStatusLineRegister')
                .. vim.g[statusline_reg]
                .. reset_colour
end

local function sl_time()
        if vim.g[statusline_time] ~= '' then
                return make_hg('CustomStatusLineTime')
                        .. string.format('%x%d', os.date('*t').hour % 12, os.date('*t').min)
        else
                return ''
        end
end

local function wrap_with_icon(filename, icon_type)
        local icon, hg = require('nvim-web-devicons').get_icon(icon_type)
        return {
                filename = filename,
                icon = icon,
                hg = hg or 'DevIconMojo',
                filetype = icon_type,
        }
end

local function get_file_icon_hg(filepath)
        local _, neogit_type = filepath:match('(.+)/Neogit(.+)')
        if neogit_type ~= nil then return wrap_with_icon(neogit_type, 'git') end
        local _, git_file = filepath:match('(.+)/.git/(.+)')
        if git_file ~= nil then return wrap_with_icon(git_file, 'git') end

        local _, oil_path = filepath:match('oil://(.+)')
        if oil_path ~= nil then return wrap_with_icon(reduce_path(oil_path), 'tmux') end

        local term_path = filepath:match('term://(.+)')
        if term_path ~= nil then return wrap_with_icon('>_', 'sh') end

        local local_file_name = reduce_path(filepath)
        -- 	local local_file_name = reduce_path(vim.fn.expand('%:t'))
        local ext = vim.fn.fnamemodify(local_file_name, ':e')
        if ext == '' then ext = vim.bo.filetype end

        return wrap_with_icon(local_file_name, ext)
end

------------------
--- Treesitter ---
------------------

local function nodext(child) return vim.treesitter.get_node_text(child, 0) end

local function verbose() return vim.g[statusline_full_treesitter] ~= '' end

local function ins(ns, vs, hg)
        if hg == nil then hg = '@keyword' end
        if verbose() and vs ~= nil then
                vs = string.gsub(vs, '%s+', ' ')
                if #vs >= 30 then vs = string.sub(vs, 1, 28) .. '..' end
                return make_hg(hg) .. ns .. '{' .. make_hg('Comment') .. vs .. make_hg(hg) .. '}'
        else
                return make_hg(hg) .. ns
        end
end

local debug_ts = false

local function find_concat(node, names, sep)
        local txt = ''
        for child in node:iter_children() do
                if
                        verbose() and vim.tbl_contains(names, child:type())
                        or child:type() == names[1]
                then
                        if txt ~= '' and sep then txt = txt .. sep end
                        txt = txt .. nodext(child)
                end
        end
        return string.gsub(txt, '%s+', ' ')
end

local function sl_display_node_lua(node, ft)
        -- sometimes positional because name can change, sometimes name because position can change
        local ty = node:type()
        if not ty then return '' end

        local lua = ft == 'lua'
        local rs = ft == 'rs'
        local py = ft == 'py'

        --- keywords ---
        local ones = {
                lua = { 'for_statement', 'if_statement', 'elseif_statement', 'while_statement' },
                rs = { 'match_expression', 'while_expression' },
                py = { 'while_statement', 'with_statement', 'except_clause' },
        }
        ones = ones[ft]
        if ones ~= nil then
                for _, val in ipairs(ones) do
                        if ty == val then
                                local name = string.match(val, '^([^_]*)')
                                return ins(name, nodext(node:child(1)))
                        end
                end
        end
        -- py
        if py and ty == 'try_statement' then return make_hg('@keyword') .. 'try' end
        if py and ty == 'finally_clause' then return make_hg('@keyword') .. 'finally' end
        -- rs
        if rs and ty == 'for_expression' or py and ty == 'for_statement' then
                return ins('for', nodext(node:child(1)) .. ' in ' .. nodext(node:child(3)))
        end
        if rs then
                if ty == 'loop_expression' then
                        return make_hg('@keyword') .. 'loop'
                elseif ty == 'mod_item' then
                        return ins('mod', nodext(node:child(1)), '@module')
                end
                for _, x in ipairs({ 'struct', 'trait', 'enum' }) do
                        if ty == x .. '_item' then
                                local vb =
                                        find_concat(node, { 'type_identifier', 'type_parameters' })
                                return ins(x, vb)
                        end
                end

                if ty == 'impl_item' then
                        local tys = { 'type_identifier', 'generic_type' }
                        return ins('impl', find_concat(node, tys, ' for '))
                end
        end
        if py then
                if ty == 'class_definition' then
                        return ins(
                                'class',
                                find_concat(node, { 'identifier', 'argument_list' }),
                                '@keyword'
                        )
                end
        end

        --- assignments ---
        if lua and ty == 'assignment_statement' then
                return make_hg('@variable') .. nodext(node:child(0):child(0))
        end
        --- functions ---
        local fn = make_hg('@function')
        local comm = make_hg('Comment')
        local ret = fn .. '  ' .. comm
        local tys = { 'identifier', 'parameters' }
        if lua and ty == 'function_declaration' then
                return fn .. string.gsub(find_concat(node, tys, comm), '%s+', '')
        end

        if rs and ty == 'function_item' then
                local txt = fn .. find_concat(node, tys, comm)
                if verbose() then
                        txt = fn
                                .. '{'
                                .. txt
                                .. ret
                                .. nodext(node:child(node:child_count() - 2))
                                .. fn
                                .. '}'
                end
                return txt
        end
        if py and ty == 'function_definition' then
                local rety = find_concat(node, { 'type' })
                local txt = fn .. find_concat(node, tys, comm)
                if rety ~= '' and verbose() then
                        txt = fn .. '{' .. txt .. ret .. rety .. fn .. '}'
                end
                return txt
        end
        if py and ty == 'call' then return fn .. nodext(node:child(0)) .. '°' end
        return ''
end

local function sl_display_node(node, ft)
        local type = node:type()

        local txt = sl_display_node_lua(node, ft)
        txt = vim.fn.substitute(txt, '()', '', 'g')

        if txt ~= '' or not debug_ts then return txt end

        local names = {}
        table.insert(names, type)
        for child in node:iter_children() do
                table.insert(names, child:type())
        end
        return make_hg('none') .. string.sub(table.concat(names, '|'), 1, 100)
end

------------
--- Main ---
------------

local function sl_file_part(is_active)
        -- filetype and icons
        local file_icon_hg = get_file_icon_hg(get_displayed_file_name())
        local sep = sl_sep()
        local hg = make_hg('CustomStatusLineFilename')
        local file = sep .. hg .. file_icon_hg.filename .. reset_colour

        -- inactive buffer
        if not is_active then return file end

        -- oil
        local node = vim.treesitter.get_node()
        if vim.g[statusline_treesitter] == '' or not node then return file end

        -- ast
        local path = ''
        while node do
                local new = sl_display_node(node, file_icon_hg.filetype)
                path = new .. path
                if new ~= '' then
                        path = sep .. path
                        if debug_ts then break end
                end
                node = node:parent()
        end

        return file .. path
end

function _G.___custom_statusline_content()
        local is_active = vim.g.statusline_winid == vim.fn.win_getid()
        local line = ''
        if is_active then line = line .. sl_time() end
        line = line .. sl_file_part(is_active)
        if is_active then line = line .. '%=' .. sl_position() .. sl_messages() .. sl_battery() end
        return line
end

vim.o.statusline = '%!v:lua.___custom_statusline_content()'
