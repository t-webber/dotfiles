local keymaps = {}

local describe = function(description, action)
	local t = type(description)
	if t == "string" then
		return description
	end
	if description then
		error("unsupported description " .. description)
	end
	if type(action) == "string" then
		return action:gsub("%<CR>", "󰌑 "):gsub("%<Esc>", "󱊷 ")
	end
	return ""
end

local setm = function(modes, keymap, action, description)
	for _, mode in ipairs(modes) do
		vim.keymap.set(mode, keymap, action, {})
	end
	if keymaps[keymap] ~= nil then
		error("multiple keymaps for " .. keymap)
	end
	keymaps[keymap] = { description = describe(description, action), modes = modes }
end
local sets = function(mode, keymap, action, description)
	setm({ mode }, keymap, action, description)
end

local n = "n"
local v = "v"
local i = "i"
local t = "t"
local ni = { n, i }
local nv = { n, v }

--------------
--- Global ---
--------------

setm(nv, "<C-y>", '"+y')
sets(n, "<C-a>", "gg^vG$<CR>")
setm(ni, "<C-S>", "<Esc>:w<CR>")
setm(ni, "<C-x>", "<Esc>:w<CR>:so<CR>")

sets(n, "gu", "1G^")
sets(n, "gd", "G$")
sets(t, "<Esc>", [[<C-\><C-n>]])

sets(n, "gh", function()
	print(vim.fn.synIDattr(vim.fn.synID(vim.fn.line("."), vim.fn.col("."), 1), "name"))
end, "Current highlight group")

---------------
--- Windows ---
---------------

sets(n, "<C-h>", "<C-w>h")
sets(n, "<C-j>", "<C-w>j")
sets(n, "<C-k>", "<C-w>k")
sets(n, "<C-l>", "<C-w>l")

-----------
--- Git ---
-----------

sets(n, "gp", ":Gitsigns preview_hunk<CR>")
sets(n, "gi", ":Gitsigns preview_hunk_inline<CR>")
sets(n, "gb", ":Gitsigns toggle_current_line_blame<CR>")
sets(n, "ga", ":term git add -p<CR>")
sets(n, "go", ":Neogit<CR>")

-----------------
--- Telescope ---
-----------------

sets(n, "èf", ":Telescope find_files<CR>")
sets(n, "èg", ":Telescope git_files<CR>")
sets(n, "èz", ":Telescope grep_string<CR>")
sets(n, "èd", ":Telescope git_status<CR>")

-----------
--- Oil ---
-----------

sets(n, "èh", ":Oil<CR>")
sets(n, "èl", function()
	local path = vim.fn.expand("<cfile>")
	require("oil").open(path)
end, "Opens the file")

-----------
--- LSP ---
-----------

--> errors <--

sets(n, "àw", function()
	vim.diagnostic.open_float()
end, "Open float")
sets(n, "àf", function()
	vim.lsp.buf.code_action()
end, "Code action")

local move_and_open = function(action)
	return function()
		vim.diagnostic.setqflist({ open = false })
		vim.cmd(action)
		vim.defer_fn(function()
			vim.diagnostic.open_float(nil, { focus = false })
		end, 50)
	end
end

sets(n, "àl", move_and_open("cnext"), "cnext")
sets(n, "àh", move_and_open("cprev"), "cprev")
sets(n, "àk", ":cclose<CR>", "cclose")
sets(n, "àj", ":copen<CR>", "copen")

--> motion <--

sets(n, "àd", function()
	vim.lsp.buf.definition({})
end, "Go to definition")

------------
--- Case ---
------------

-- press ~ to toggle a letter between lower case and capitals
setm({ n, v }, "cc", ":TtCamel<CR>")
setm({ n, v }, "cp", ":TtPascal<CR>")
setm({ n, v }, "ck", ":TtKebab<CR>")
setm({ n, v }, "cs", ":TtSnake<CR>")
setm({ n, v }, "co", ":TtConst<CR>")
setm({ n, v }, "cd", ":TtDot<CR>")
setm({ n, v }, "ct", ":TtTitle<CR>")
vim.keymap.set(v, "c", "<Nop>")

-------------------------
--- Comment/Uncomment ---
-------------------------

sets(n, "<C-!>", function()
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
end, "Comment/Uncomment line")

sets(n, "àp", function()
	local clients = vim.lsp.get_clients()
	local buf = vim.api.nvim_get_current_buf()
	for _, client in ipairs(clients) do
		if vim.lsp.get_client_by_id(client.id) and vim.lsp.buf_is_attached(buf, client.id) then
			print("Active LSP client:", client.name)
		end
	end
end, "Print active lsp clients")

---------------------
--- Custom helper ---
---------------------

local load_lines = function(width)
	local keys = {}
	for key, _ in pairs(keymaps) do
		table.insert(keys, key)
	end

	table.sort(keys)

	local lines = { "Custom keymaps (q to exit)", string.rep("-", width) }

	for _, key in ipairs(keys) do
		local modes = table.concat(keymaps[key].modes)
		local description = keymaps[key].description or "nil"
		local line = key .. " \t" .. modes .. "\t " .. description
		table.insert(lines, line)
	end

	return lines
end

vim.api.nvim_set_keymap(n, "g?", "<Nop>", {
	callback = function()
		local ui = vim.api.nvim_list_uis()[1]
		local width = 60

		local lines = load_lines(width)

		local height = #lines
		local col = math.floor((ui.width - width) / 2)
		local row = math.floor((ui.height - height) / 2)

		local buf = vim.api.nvim_create_buf(false, true)
		vim.api.nvim_buf_set_lines(buf, 0, -1, false, lines)

		local opts = {
			style = "minimal",
			relative = "editor",
			width = width,
			height = height,
			row = row,
			col = col,
			border = "rounded",
		}

		vim.api.nvim_open_win(buf, true, opts)

		vim.api.nvim_buf_set_keymap(buf, "n", "q", ":close<CR>", { noremap = true, silent = true })
	end,
})
