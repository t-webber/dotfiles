--------------------------
--- External utilities ---
--------------------------

local v = require('globals')
local statusline_notif = v.statusline_notif
local statusline_notif_enable = v.statusline_notif_enable
local statusline_reg = v.statusline_reg
local statusline_linecol = v.statusline_linecol
local statusline_filepath = v.statusline_filepath
local statusline_battery = v.statusline_battery
local statusline_time = v.statusline_time

package.loaded['scopes'] = nil
local sl_ts_display = require('scopes')

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

----------------------------
--- Depend on global var ---
----------------------------

local function extract_dir_file(abs_path, dir)
        local parts = vim.split(abs_path, '/')
        local nb = #parts
        if nb <= 1 then return abs_path end
        if dir then return parts[nb - 1] .. '/' .. parts[nb] end
        return parts[nb]
end

local function get_displayed_file_name()
        local display = vim.g[statusline_filepath]
        local abs_path = get_abs_path()
        if display == 'path' then return abs_path end
        if display == 'dir' then return extract_dir_file(abs_path, true) end
        if display == 'file' then return extract_dir_file(abs_path, false) end
        if display == '' then return '' end
        return '!!!' .. display .. '!!!'
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
        if vim.g[statusline_time] == '' then return '' end
        return make_hg('CustomStatusLineTime')
                .. string.format('%x%d', os.date('*t').hour % 12, os.date('*t').min)
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

local function sl_file_part(is_active)
        -- filetype and icons
        local file_icon_hg = get_file_icon_hg(get_displayed_file_name())
        local sep = sl_sep()
        local hg = make_hg('CustomStatusLineFilename')
        local file = sep .. hg .. file_icon_hg.filename .. reset_colour

        -- inactive buffer
        if not is_active then return file end

        -- oil
        return file .. sl_ts_display(file_icon_hg.filetype, sep)
end

------------
--- Main ---
------------

function _G.___custom_statusline_content()
        local is_active = vim.g.statusline_winid == vim.fn.win_getid()
        local line = ''
        if is_active then line = line .. sl_time() end
        line = line .. sl_file_part(is_active)
        if is_active then line = line .. '%=' .. sl_position() .. sl_messages() .. sl_battery() end
        return line
end

vim.o.statusline = '%!v:lua.___custom_statusline_content()'
