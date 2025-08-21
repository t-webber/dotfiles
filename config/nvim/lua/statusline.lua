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
	return mode_names[vim.fn.mode()] or "Unknown" -- mode can be nil when transitioning (e.g. in Neogit)
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
	local sep = " │ "
	if eol ~= nil then
		sep = " │"
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
		.. "%#CustomStatusLineHardtime#"
		.. vim.g[statusline_notif]
		.. "%#CustomStatusLineDefault#"
		.. outer_sep
end

local sl_time = function()
	return string.format(" %x%d", os.date("*t").hour % 12, os.date("*t").min)
end

local replace_prefix = function(path, from, to)
	if subpath ~= nil then
		return to .. subpath
	end
end

local reduce_path = function(path)
	local subpath
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
	return path
end

local wrap_with_icon = function(filename, icon_type)
	local icon, hg = require("nvim-web-devicons").get_icon(icon_type)
	return { filename = filename, icon = icon, hg = hg }
end

local get_file_icon_hg = function()
	local winid = vim.g.statusline_winid
	local bufnr = vim.api.nvim_win_get_buf(winid)
	local filepath = vim.api.nvim_buf_get_name(bufnr)
	local tabnr = vim.api.nvim_win_get_tabpage(winid)
	-- 	if true then
	-- 		return { filename = filepath, hg = "", icon = "" }
	-- 	end

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

local sl_filetype = function()
	local fih = get_file_icon_hg()
	return "%#"
		.. fih.hg
		.. "#"
		.. fih.filename
		.. " "
		.. fih.icon
		.. "%#CustomStatusLineDefault#"
end

function _G.___custom_statusline_content()
	return sl_mode()
		.. sl_time()
		.. sl_sep()
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
