local set = vim.keymap.set
local setm = function(modes, keybind, action)
	for _, mode in ipairs(modes) do
		set(mode, keybind, action)
	end
end
local n = "n"
local v = "v"
local i = "i"
local t = "t"
local ni = { n, i }

--------------
--- Global ---
--------------

set(v, "<C-y>", '"+y<CR>')
set(n, "<C-y>", '"+y')
set(n, "<C-a>", "gg^vG$<CR>")
setm(ni, "<C-S>", "<Esc>:w<CR>")
setm(ni, "<C-x>", "<Esc>:w<CR>:so<CR>")
setm(ni, "<C-q>q", "<Esc>:q<CR>")

set(n, "gt", "1G^")
set(n, "gb", "G$")
set(t, "<Esc>", [[<C-\><C-n>]])

vim.keymap.set("n", " hg", function()
	print(vim.fn.synIDattr(vim.fn.synID(vim.fn.line("."), vim.fn.col("."), 1), "name"))
end)

---------------
--- Windows ---
---------------

set(n, "<C-h>", "<C-w>h")
set(n, "<C-j>", "<C-w>j")
set(n, "<C-k>", "<C-w>k")
set(n, "<C-l>", "<C-w>l")

-----------
--- Git ---
-----------

set(n, ",gh", ":Gitsigns preview_hunk<CR>")
set(n, ",gi", ":Gitsigns preview_hunk_inline<CR>")
set(n, ",gb", ":Gitsigns toggle_current_line_blame<CR>")
set(n, ",ga", ":term git add -p<CR>")
set(n, ",gg", ":Neogit<CR>")

-----------------
--- Telescope ---
-----------------

set(n, ",tf", ":Telescope find_files<CR>")
set(n, ",tg", ":Telescope git_files<CR>")
set(n, ",tz", ":Telescope grep_string<CR>")
set(n, ",td", ":Telescope git_status<CR>")

-----------
--- Oil ---
-----------

set(n, ",oh", ":Oil<CR>")
set(n, ",ol", ":Oil ..<CR>")

-----------
--- LSP ---
-----------

--> errors <--

set(n, ",lw", function()
	vim.diagnostic.open_float()
end)
set(n, ",lf", function()
	vim.lsp.buf.code_action()
end)

local move_and_open = function(action)
	return function()
		vim.diagnostic.setqflist({ open = false })
		vim.cmd(action)
		vim.defer_fn(function()
			vim.diagnostic.open_float(nil, { focus = false })
		end, 50)
	end
end

set(n, ",ll", move_and_open("cnext"))
set(n, ",lh", move_and_open("cprev"))
set(n, ",lk", ":cclose<CR>")
set(n, ",lj", ":copen<CR>")

--> motion <--

set(n, ",ld", function()
	vim.lsp.buf.definition({})
end)

------------
--- Case ---
------------

-- press ~ to toggle a letter between lower case and capitals
setm({ n, v }, ",cc", ":TtCamel<CR>")
setm({ n, v }, ",cp", ":TtPascal<CR>")
setm({ n, v }, ",ck", ":TtKebab<CR>")
setm({ n, v }, ",cs", ":TtSnake<CR>")
setm({ n, v }, ",co", ":TtConst<CR>")
setm({ n, v }, ",cd", ":TtDot<CR>")
setm({ n, v }, ",ct", ":TtTitle<CR>")
set(v, "c", "<Nop>")

-------------------------
--- Comment/Uncomment ---
-------------------------

set(n, "<C-!>", function()
	print("xxx")
	local comments = vim.bo.comments
	local comment = comments:match("([^:]+)$")
	local pattern = "^%s*(" .. vim.pesc(comment) .. "+)%s*"

	local line = vim.api.nvim_get_current_line()

	local _, _, match = line:find(pattern)

	if match then
		line = line:gsub(pattern, "")
	else
		line = comment .. " " .. line
	end

	vim.api.nvim_set_current_line(line)
	vim.cmd("normal! j")
end)

set(n, "gtf", function()
	local clients = vim.lsp.get_clients()
	local buf = vim.api.nvim_get_current_buf()
	for _, client in ipairs(clients) do
		if vim.lsp.get_client_by_id(client.id) and vim.lsp.buf_is_attached(buf, client.id) then
			print("Active LSP client:", client.name)
		end
	end
end)
