local v = require("globals")
local statusline_notif = v.statusline_notif
local statusline_reg = v.statusline_reg

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

local get_mode_name = function()
	return mode_names[vim.fn.mode()]
end

local sl_mode = function()
	local mode = get_mode_name()
	return "%#CustomStatusLine"
		.. mode
		.. "# "
		.. mode
		.. " %#CustomStatusLineDefault#"
end

local sl_sep = function(eol)
	local mode = get_mode_name()
	local sep = " | "
	if eol ~= nil then
		sep = " |"
	end
	return "%#CustomStatusLine"
		.. mode
		.. "Symbol#"
		.. sep
		.. "%#CustomStatusLineDefault#"
end

local sl_battery = function()
	local file = io.open("/sys/class/power_supply/BAT1/capacity", "r")
	if not file then
		return ""
	end
	local content = file:read("*l")
	file:close()
	local level = tonumber(content)
	if level < 5 then
		local b = vim.api.nvim_create_buf(false, true)
		vim.api.nvim_buf_set_lines(b, 0, -1, false, { "Battery very low" })
		vim.api.nvim_buf_set_option(b, "modifiable", false)
		vim.api.nvim_open_win(b, true, {
			relative = "editor",
			width = 2,
			height = 1,
			row = 10,
			col = 10,
			style = "minimal",
			border = "single",
		})
	end
	return sl_sep() .. string.format("%s%%%%", content)
end

local sl_position = function()
	local line = vim.fn.line(".")
	local col = vim.fn.col(".")
	return string.format("%d:%d", line, col)
end

local sl_messages = function()
	local inner_sep = ""
	local outer_sep = ""

	if vim.g[statusline_reg] ~= "" and vim.g[statusline_notif] ~= "" then
		inner_sep = sl_sep()
		outer_sep = inner_sep
	elseif vim.g[statusline_reg] ~= "" or vim.g[statusline_notif] ~= "" then
		outer_sep = sl_sep()
	end
	return "%#CustomStatusLineRegister#"
		.. vim.g[statusline_reg]
		.. inner_sep
		.. "%#CustomStatusLineDefault#"
		.. vim.g[statusline_notif]
		.. outer_sep
end

local sl_time = function()
	return string.format(" %x%d", os.date("*t").hour, os.date("*t").min)
end

local sl_filetype = function()
	local devicons = require("nvim-web-devicons")
	local filetype = vim.bo.filetype
	local icon, hg = devicons.get_icon(filetype)
	return "%#" .. hg .. "#" .. icon .. "%#CustomStatusLineDefault#"
end

function _G.___custom_statusline_content()
	return sl_mode()
		.. sl_time()
		.. sl_sep()
		.. "%f "
		.. sl_filetype()
		.. sl_sep()
		.. sl_messages()
		.. "%="
		.. sl_battery()
		.. sl_sep()
		.. sl_position()
		.. sl_sep(1)
end

vim.o.statusline = "%!v:lua.___custom_statusline_content()"
