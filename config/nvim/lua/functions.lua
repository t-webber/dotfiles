vim.g.lualine_notification = ""
local core_notify = vim.notify
vim.notify = function(msg, ...)
	if type(msg) ~= "string" then
		error("found non string notification")
	end
	local key = msg:match("^You pressed the (.+) key too soon!")
	if key then
		vim.g.lualine_notification = "You pressed " .. key .. " to many times!"
	elseif msg:match("instead of") or msg:match("key is disabled!") then
		vim.g.lualine_notification = msg
	else
		core_notify(msg, ...)
	end
end

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
