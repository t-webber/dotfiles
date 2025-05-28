-------------------------
--- Lualine functions ---
-------------------------

local v = require("globals")
local lualine_notif = v.lualine_notif
local lualine_reg = v.lualine_reg

vim.g[lualine_notif] = ""
local core_notify = vim.notify
vim.notify = function(msg, ...)
	if type(msg) ~= "string" then
		error("found non string notification")
	end
	local key = msg:match("^You pressed the (.+) key too soon!")
	if key then
		vim.g[lualine_notif] = "You pressed " .. key .. " to many times!"
		vim.cmd("redrawstatus")
	elseif msg:match("instead of") or msg:match("key is disabled!") then
		vim.g[lualine_notif] = msg
		vim.cmd("redrawstatus")
	else
		core_notify(msg, ...)
	end
end

vim.api.nvim_create_autocmd({ "RecordingEnter" }, {
	callback = function()
		local reg = vim.fn.reg_recording()
		if reg == "" then
			vim.g[lualine_reg] = ""
		else
			vim.g[lualine_reg] = "Recording @" .. reg
		end
		require("lualine").refresh()
	end,
})

vim.api.nvim_create_autocmd({ "RecordingLeave" }, {
	callback = function()
		vim.g[lualine_reg] = ""
		require("lualine").refresh()
	end,
})

-----------------------
--- Other functions ---
-----------------------

vim.api.nvim_create_autocmd("TextYankPost", {
	desc = "Highlight when yanking (copying) text",
	callback = function()
		vim.hl.on_yank()
	end,
})

vim.api.nvim_create_user_command("GitBlameLine", function()
	local line_number = vim.fn.line(".") -- Get the current line number. See `:h line()`
	local filename = vim.api.nvim_buf_get_name(0)
	print(vim.fn.system({ "git", "blame", "-L", line_number .. ",+1", filename }))
end, { desc = "Print the git blame for the current line" })
