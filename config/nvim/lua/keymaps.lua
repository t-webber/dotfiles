local api = vim.api
local fn = vim.fn
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
	return "?"
end

local setk = function(modes, keymap, action, description)
	for _, mode in ipairs(modes) do
		vim.keymap.set(mode, keymap, action, {})
	end
	if keymaps[keymap] ~= nil then
		error("multiple keymaps for " .. keymap)
	end
	keymaps[keymap] = { description = describe(description, action), modes = modes }
end

local uset = function(modes, keymap)
	for _, mode in ipairs(modes) do
		vim.keymap.set(mode, keymap, "<Nop>")
	end
end

local n = { "n" }
local i = { "i" }
local v = { "v" }
local t = { "t" }
local ni = { n, i }
local nv = { n, v }

--------------
--- Global ---
--------------

setk(nv, "<C-y>", '"+y')
setk(n, "<C-a>", "gg^vG$<CR>")
setk(ni, "<C-s>", "<Esc>:w<CR>")
setk(ni, "<C-x>", "<Esc>:w<CR>:so<CR>")

setk(t, "<Esc>", [[<C-\><C-n>]])

setk(n, "gh", function()
	print(fn.synIDattr(fn.synID(fn.line("."), fn.col("."), 1), "name"))
end, "Current highlight group")

setk(n, "<C-n>", ":enew | setlocal nonu nornu nomodifiable<CR>", "Open empty immuable buffer")

setk(n, "è ", ":noh<CR>")

----------------
--- Windows ---
----------------

setk(n, "<C-h>", "<C-w>h")
setk(n, "<C-j>", "<C-w>j")
setk(n, "<C-k>", "<C-w>k")
setk(n, "<C-l>", "<C-w>l")

-----------------
--- Iron REPL ---
-----------------

setk(n, "è:", ":IronAttach<CR>")
setk(n, "è!", ":IronHide<CR>")
setk(n, "<C-è>", function()
	local line = api.nvim_get_current_line()
	print(line)
	if line == "" then
		line = " "
	end
	api.nvim_command("IronSend " .. line:gsub(" ", "\\ "))

	api.nvim_command("normal! j")
end)

---------------
--- Buffers ---
---------------

setk(n, "<C-*>", ":vsplit | term<CR>")
setk(n, "<C-ù>", ":split | term<CR>")

-----------
--- Git ---
-----------

setk(n, "gp", ":Gitsigns preview_hunk<CR>")
setk(n, "gi", ":Gitsigns preview_hunk_inline<CR>")
setk(n, "gb", ":Gitsigns toggle_current_line_blame<CR>")
setk(n, "ga", ":te git add -p<CR>")
setk(n, "go", ":Neogit<CR>")

-----------------
--- Telescope ---
-----------------

setk(n, "éf", ":Telescope find_files<CR>")
setk(n, "ég", ":Telescope git_files<CR>")
setk(n, "és", ":Telescope git_status<CR>")
setk(n, "éh", ":Telescope highlights<CR>")
setk(n, "ée", ":Telescope live_grep<CR>")
setk(n, "ér", ":Telescope registers<CR>")

-----------
--- Oil ---
-----------

setk(n, "èd", ":Oil " .. (os.getenv("DEV") or "") .. "<CR>")
setk(n, "èf", ":Oil " .. (os.getenv("CONFIG") or "") .. "<CR>")
setk(n, "èc", ":Oil " .. (os.getenv("CMD") or "") .. "<CR>")
setk(n, "èk", ":Oil<CR>")
setk(n, "èj", function()
	local path = fn.expand("<cfile>")
	require("oil").open(path)
end, "Opens the file")

-----------
--- LSP ---
-----------

--> errors <--

setk(n, "àw", function()
	vim.diagnostic.open_float()
end, "Open float")
setk(n, "àf", function()
	vim.lsp.buf.code_action()
end, "Code action")

local move_and_open = function(action)
	return function()
		vim.diagnostic.setqflist({ open = false })
		api.nvil_command(action)
		vim.defer_fn(function()
			vim.diagnostic.open_float(nil, { focus = false })
		end, 50)
	end
end

setk(n, "àl", move_and_open("cnext"), "cnext")
setk(n, "àh", move_and_open("cprev"), "cprev")
setk(n, "àk", ":cclose<CR>", "cclose")
setk(n, "àj", ":copen<CR>", "copen")

--> motion <--

setk(n, "àd", function()
	vim.lsp.buf.definition({})
end, "Go to definition")

--> lsp status <--

setk(n, "às", ":checkhealth lsp<CR>")

setk(n, "àp", function()
	local clients = vim.lsp.get_clients()
	local buf = api.nvim_get_current_buf()
	for _, client in ipairs(clients) do
		if vim.lsp.get_client_by_id(client.id) and vim.lsp.buf_is_attached(buf, client.id) then
			print("Active LSP client:", client.name)
		end
	end
end, "Print active lsp clients")

------------
--- Case ---
------------

-- press ~ to toggle a letter between lower case and capitals
setk(nv, "cc", ":TtCamel<CR>")
setk(nv, "cp", ":TtPascal<CR>")
setk(nv, "ck", ":TtKebab<CR>")
setk(nv, "cs", ":TtSnake<CR>")
setk(nv, "co", ":TtConst<CR>")
setk(nv, "cd", ":TtDot<CR>")
setk(nv, "ct", ":TtTitle<CR>")

uset(nv, "c")

-------------------------
--- Comment/Uncomment ---
-------------------------

setk(n, "<C-!>", function()
	local comments = vim.bo.comments
	local comment = comments:match("([^:]+)$")
	local pattern = "^%s*(" .. vim.pesc(comment) .. "+)%s*"

	local line = api.nvim_get_current_line()

	local _, _, match = line:find(pattern)

	if match then
		line = line:gsub(pattern, "")
	else
		line = comment .. " " .. line
	end

	api.nvim_set_current_line(line)
	vim.cmd("normal! j")
end, "Comment/Uncomment line")

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
		local line = key:gsub(" ", "␣") .. " \t" .. modes .. "\t " .. description
		table.insert(lines, line)
	end

	return lines
end

api.nvim_set_keymap("n", "g?", "<Nop>", {
	callback = function()
		local ui = api.nvim_list_uis()[1]
		local width = 60

		local lines = load_lines(width)

		local height = #lines
		local col = math.floor((ui.width - width) / 2)
		local row = math.floor((ui.height - height) / 2)

		local buf = api.nvim_create_buf(false, true)
		api.nvim_buf_set_lines(buf, 0, -1, false, lines)

		local opts = {
			style = "minimal",
			relative = "editor",
			width = width,
			height = height,
			row = row,
			col = col,
			border = "rounded",
		}

		local win = api.nvim_open_win(buf, true, opts)
		api.nvim_win_set_option(win, "winhl", "Normal:Normal")

		api.nvim_buf_set_keymap(buf, "n", "q", ":close<CR>", { noremap = true, silent = true })
	end,
})
