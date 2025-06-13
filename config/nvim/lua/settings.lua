local o = vim.opt

o.relativenumber = true
o.number = true -- absolute nb for current line
o.cursorline = true -- needed to customise even only cursorlinenr
o.cmdheight = 0 -- hide command line
o.mouse = "" -- disable mouse
o.showmode = false -- hide `--INSERT--` in command line
o.fillchars = { eob = " " }
o.clipboard:append("unnamedplus")

local g = vim.g

g.c_syntax_for_h = 1 -- consider .h files as .c files
g.loaded_node_provider = 0
g.loaded_perl_provider = 0
g.loaded_python3_provider = 0
g.loaded_ruby_provider = 0

-- show inline errors
vim.diagnostic.config({ virtual_text = true })

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
