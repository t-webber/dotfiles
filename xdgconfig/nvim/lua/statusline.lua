local v = require("globals")
local statusline_notif = v.statusline_notif
local statusline_reg = v.statusline_reg

local fullfilepath = vim.fn.expand("%:p")
local dot = os.getenv("DOT")
if dot == nil then
	vim.g[statusline_reg] = "No $DOT..."
elseif fullfilepath:find(dot, 1, true) then
	vim.g[statusline_reg] = "Don't forget to pull..."
end

local function make_hg(hg_name)
	return "%#" .. hg_name .. "#"
end

local reset_colour = make_hg("CustomStatusLineDefault")

local mode_names = {
	n = "Normal",
	i = "Insert",
	v = "Visual",
	V = "VisualLine",
	["\22"] = "VisualBlock",
	["\19"] = "SelectBlock",
	s = "Select",
	S = "SelectLine",
	c = "Command",
	R = "Replace",
	t = "Terminal",
}

local function get_mode_name()
	return mode_names[vim.fn.mode()] or "Unknown" -- mode can be nil when transitioning (e.g. in Neogit)
end

-- local function sl_mode()
-- 	local mode = get_mode_name()
-- 	return "%#CustomStatusLine"
-- 		.. mode
-- 		.. "# "
-- 		.. mode
-- 		.. reset_colour
-- end

local function sl_sep(char)
	if char == nil then
		char = "|"
	end
	local mode = get_mode_name()
	local sep = "" .. char .. ""
	return make_hg("CustomStatusLine" .. mode .. "Symbol")
		.. sep
		.. reset_colour
end

local function sl_battery()
	local file = io.open("/sys/class/power_supply/BAT1/capacity", "r")
	if not file then
		return ""
	end
	local content = file:read("*l")
	file:close()
	return sl_sep() .. string.format("%s%%%%", content)
end

local function sl_position()
	local line = vim.fn.line(".")
	local col = vim.fn.col(".")
	return string.format("%d:%d", line, col)
end

local function sl_messages()
	local sep = ""

	if vim.g[statusline_reg] ~= "" and vim.g[statusline_notif] ~= "" then
		sep = sl_sep()
	end
	return make_hg("CustomStatusLineRegister")
		.. vim.g[statusline_reg]
		.. sep
		.. make_hg("CustomStatusLineHardtime")
		.. vim.g[statusline_notif]
		.. reset_colour
end

local function sl_time()
	return string.format("%x%d", os.date("*t").hour % 12, os.date("*t").min)
end

local function reduce_path(path)
	local subpath

	subpath = path:match(os.getenv("DATA") .. "/cargo/registry/src/")
	if subpath ~= nil then
		return " " .. subpath
	end
	subpath = path:match(os.getenv("DEV") .. "/(.+)")
	if subpath ~= nil then
		return " " .. subpath
	end
	subpath = path:match(os.getenv("DOT") .. "/(.+)")
	if subpath ~= nil then
		return " " .. subpath
	end
	subpath = path:match(os.getenv("FILES") .. "/(.+)")
	if subpath ~= nil then
		return "" .. subpath
	end
	subpath = path:match((os.getenv("WORK") or "!!!!") .. "/(.+)")
	if subpath ~= nil then
		return "󰃖 " .. subpath
	end
	subpath = path:match(os.getenv("WASTE") .. "/(.+)")
	if subpath ~= nil then
		return " " .. subpath
	end
	return path
end

local function wrap_with_icon(filename, icon_type)
	local icon, hg = require("nvim-web-devicons").get_icon(icon_type)
	return {
		filename = filename,
		icon = icon,
		hg = hg or "DevIconMojo",
		filetype = icon_type,
	}
end

local function get_file_icon_hg()
	local winid = vim.g.statusline_winid
	local bufnr = vim.api.nvim_win_get_buf(winid)
	local filepath = vim.api.nvim_buf_get_name(bufnr)
	local tabnr = vim.api.nvim_win_get_tabpage(winid)

	local _, neogit_type = filepath:match("(.+)/Neogit(.+)")
	if neogit_type ~= nil then
		return wrap_with_icon(neogit_type, "git")
	end
	local _, git_file = filepath:match("(.+)/.git/(.+)")
	if git_file ~= nil then
		return wrap_with_icon(git_file, "git")
	end

	local _, cwd = pcall(function()
		return vim.fn.getcwd(tabnr)
	end)
	if not cwd then
		return wrap_with_icon(filepath, "mojo")
	end
	local filename = vim.fn.fnamemodify(filepath, ":." .. cwd)

	local _, oil_path = filename:match("oil://(.+)")
	if oil_path ~= nil then
		return wrap_with_icon(reduce_path(oil_path), "tmux")
	end

	local _, _, arm_folder_path =
		filename:match("oil(.+)://tomweb01(.+)tomweb01/popeye(.*)/$")
	if arm_folder_path ~= nil then
		return wrap_with_icon(arm_folder_path, "tmux")
	end

	local _, _, arm_file_path =
		filename:match("oil(.+)://tomweb01(.+)tomweb01/popeye(.*)")
	if arm_file_path ~= nil then
		filename = arm_file_path
	end

	local term_path = filename:match("term://(.+)")
	if term_path ~= nil then
		return wrap_with_icon(">_", "sh")
	end

	local local_file_name = reduce_path(filename)
	local ext = vim.fn.fnamemodify(local_file_name, ":e")
	if ext == "" then
		ext = vim.bo[bufnr].filetype
	end
	return wrap_with_icon(local_file_name, ext)
end

local ts_colors = {
	["@function"] = {
		"function_definition", -- python
		"function_declaration", -- lua
		"function_call", -- lua
		"variable_list", -- lua
		"function_item", -- rust
		"command", -- sh
	},
	["@constructor"] = {
		"class_definition", -- python
	},
	["@variable"] = {
		"field", --lua
		"parameters", --lua
		"field_declaration", --rust
		"variable_assignment", --sh
		"enum_variant", -- rust
	},
	["@type"] = {
		"struct_expression", --rust
		"struct_item", --rust
		"enum_item", -- rust
		"type_item", -- rust
	},
	["@module"] = {
		"mod_item", -- rust
	},
	["@label"] = {
		"scoped_identifier", -- rust (not a label, but I want label colour)
	},
}

local function get_ts_hg(type)
	for hg, node_types in pairs(ts_colors) do
		for _, node_type in ipairs(node_types) do
			if node_type == type then
				return make_hg(hg)
			end
		end
	end

	return "(" .. type .. ")"
end

local function get_non_ts_hg(file)
	if file == "oil" then
		return make_hg("DevIconOil")
	end
	return "[" .. file .. "]"
end

local colours = require("colours")
local function sl_file_part()
	local file_icon_hg = get_file_icon_hg()
	local hg = file_icon_hg.hg and make_hg(file_icon_hg.hg or "") or ""
	local sep = hg .. " > " .. reset_colour
	local file = hg .. file_icon_hg.filename .. reset_colour
	if colours[file_icon_hg.hg] == nil then
		file = "(" .. file_icon_hg.hg .. ")" .. file
	end

	-- 	local othernode = vim.treesitter.get_node_text()
	local ts_utils = require("nvim-treesitter.ts_utils")
	local node = ts_utils.get_node_at_cursor()
	if not node then
		return file
			.. sep
			.. get_non_ts_hg(file_icon_hg.filetype)
			.. file_icon_hg.filetype
			.. sep
	end

	local path = ""

	while node do
		local name_node = node:field("name")[1]
		if name_node then
			local name = ts_utils.get_node_text(name_node, 0)[1]
			local node_type = node:type()
			local node_hg = get_ts_hg(node_type)
			path = sep .. node_hg .. name .. path
		end
		node = node:parent()
	end

	return file .. path
end

function _G.___custom_statusline_content()
	return sl_time()
		.. sl_sep()
		.. sl_file_part()
		.. "%="
		.. sl_sep()
		.. sl_position()
		.. sl_sep()
		.. sl_messages()
		.. sl_battery()
end

vim.o.statusline = "%!v:lua.___custom_statusline_content()"
