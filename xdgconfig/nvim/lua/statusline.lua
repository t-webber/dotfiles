local v = require('globals')
local statusline_notif = v.statusline_notif
local statusline_reg = v.statusline_reg
local statusline_linecol = v.statusline_linecol
local statusline_filepath = v.statusline_filepath
local statusline_treesitter = v.statusline_treesitter
local statusline_full_treesitter = v.statusline_full_treesitter

local function print_dot_warning()
	local fullfilepath = vim.fn.expand('%:p')
	local dot = os.getenv('DOT')
	if dot == nil then
		vim.g[statusline_reg] = 'No $DOT...'
	elseif fullfilepath:find(dot, 1, true) then
		vim.g[statusline_reg] = "Don't forget to pull..."
	end
end

print_dot_warning()

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

package.loaded['tscol'] = nil
local ts_colours = require('tscol')

local function ts_display_node(type, name, sep)
	if
		vim.g[statusline_treesitter] == ''
		or vim.g[statusline_full_treesitter] == ''
			and ts_colours['hiddennodes'][type] == true
	then
		return ''
	end

	for hg, node_types in pairs(ts_colours) do
		local found = node_types[type]
		if found and found ~= true then name = name or found end
		if found then
			local fixed = name
			if not name then
				if vim.g[statusline_full_treesitter] == '' then
					fixed = ''
				else
					fixed = '!' .. type .. '!'
				end
			end

			return sep .. make_hg(hg) .. fixed .. reset_colour
		end
	end

	if vim.g[statusline_full_treesitter] ~= '' then
		return sep .. '(' .. type .. ')'
	else
		return ''
	end
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
	return make_hg('CustomStatusLine' .. mode .. 'Symbol')
		.. sep
		.. reset_colour
end

local function sl_battery()
	if os.getenv('DEVICE') ~= 'acer' then return '' end
	local level = read_unchecked('/sys/class/power_supply/BAT1/capacity')
	local status = read_unchecked('/sys/class/power_supply/BAT1/status')
	if tonumber(level) < 10 and status ~= 'Charging' then
		os.execte('/bin/notify-send -u critical low battery (nvim)')
	end
	local hg = make_hg('CustomStatusLineBattery' .. status)
	return sl_sep() .. hg .. string.format('%s', level)
end

local function sl_position()
	if vim.g[statusline_linecol] == '' then return '' end
	local line = vim.fn.line('.')
	local col = vim.fn.col('.')
	return make_hg('CustomStatusLinePosition')
		.. string.format('%d:%d', line, col)
end

local function sl_messages()
	local inner_sep = ''
	local outer_sep = ''

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
	if oil_path ~= nil then
		return wrap_with_icon(reduce_path(oil_path), 'tmux')
	end

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
	local ts_utils = require('nvim-treesitter.ts_utils')
	local node = ts_utils.get_node_at_cursor()
	if not node then
		return file .. sep .. hg .. file_icon_hg.filetype .. sep
	end

	-- ast
	local path = ''

	while node do
		local name_node = node:field('name')[1]
		local name = name_node and ts_utils.get_node_text(name_node, 0)[1]
			or nil
		path = ts_display_node(node:type(), name, sep) .. path
		node = node:parent()
	end

	return file .. path
end

function _G.___custom_statusline_content()
	local is_active = vim.g.statusline_winid == vim.fn.win_getid()
	local sline = sl_time() .. sl_file_part(is_active)
	if is_active then
		return sline .. '%=' .. sl_position() .. sl_messages() .. sl_battery()
	else
		return sline
	end
end

vim.o.statusline = '%!v:lua.___custom_statusline_content()'
