-------------------------
--- Lualine functions ---
-------------------------

local v = require("globals")
local statusline_notif = v.statusline_notif
local statusline_reg = v.statusline_reg

vim.g[statusline_notif] = ""
local core_notify = vim.notify
vim.notify = function(msg, ...)
	if type(msg) ~= "string" then
		error("found non string notification")
	end
	local key = msg:match("^You pressed the (.+) key too soon!")
	if key then
		vim.g[statusline_notif] = "Don't use " .. key
		vim.cmd("redrawstatus")
		return
	end
	key = msg:match("The (.+) key is disabled!")
	if key then
		if key == "<Up>" then
			key = ""
		elseif key == "<Left>" then
			key = ""
		elseif key == "<Right>" then
			key = ""
		elseif key == "<Down>" then
			key = ""
		end
		vim.g[statusline_notif] = key .. " disabled"
		vim.cmd("redrawstatus")
		return
	end
	local first, second = msg:match("Use (.+) instead of (.+)")
	if first and second then
		vim.g[statusline_notif] = first .. "  " .. second
		vim.cmd("redrawstatus")
		return
	end
	core_notify("core notif: " .. msg, ...)
end

vim.api.nvim_create_autocmd({ "RecordingEnter" }, {
	callback = function()
		local reg = vim.fn.reg_recording()
		if reg == "" then
			vim.g[statusline_reg] = ""
		else
			vim.g[statusline_reg] = reg
		end
	end,
})

vim.api.nvim_create_autocmd({ "RecordingLeave" }, {
	callback = function()
		vim.g[statusline_reg] = ""
	end,
})

-----------------------
--- Other functions ---
-----------------------

local better_prisma_format = os.getenv("CMD") .. "/sh/paff"

vim.api.nvim_create_autocmd("BufWritePost", {
	pattern = "*.prisma",
	callback = function()
		local start = vim.loop.hrtime()

		os.execute(better_prisma_format)

		local paff = vim.loop.hrtime()

		local path = vim.api.nvim_buf_get_name(0)
		vim.cmd("bdelete!")
		vim.cmd("edit " .. vim.fn.fnameescape(path))
		vim.cmd("filetype detect")

		local crazy_shit = vim.loop.hrtime()

		local paff_time = math.floor((paff - start) / 1e6)
		local crazy_shit_time = math.floor((crazy_shit - paff) / 1e6)
		vim.g[statusline_notif] = paff_time .. " & " .. crazy_shit_time
	end,
})

vim.api.nvim_create_autocmd("TextYankPost", {
	callback = function()
		vim.hl.on_yank()
	end,
})

vim.api.nvim_create_user_command("GitBlameLine", function()
	local line_number = vim.fn.line(".") -- Get the current line number. See `:h line()`
	local filename = vim.api.nvim_buf_get_name(0)
	print(
		vim.fn.system({ "git", "blame", "-L", line_number .. ",+1", filename })
	)
end, {})
