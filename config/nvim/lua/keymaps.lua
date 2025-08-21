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

local nb = 0

local setk = function(modes, keymap, action, description, ignore)
	for _, mode in ipairs(modes) do
		vim.keymap.set(mode, keymap, action, {})
		nb = nb + 1
	end
	if ignore == nil and keymaps[keymap] ~= nil then
		error("multiple keymaps for " .. keymap)
	end
	if ignore == nil then
		keymaps[keymap] =
			{ description = describe(description, action), modes = modes }
	end
end

local n = { "n" }
local it = { "i", "t" }
local t = { "t" }
local ni = { "n", "i" }
local nv = { "n", "v" }
local nivt = { "n", "i", "v", "t" }
local nit = { "n", "i", "t" }

--------------
--- Global ---
--------------

setk(nivt, "<A-(>", "{")
setk(nivt, "<A-)>", "}")
setk(nivt, "<D-:>", "\\")
setk(nivt, "<D-(>", "[")
setk(nivt, "<D-)>", "]")
setk(nivt, "<D-é>", "~")

setk(nv, "<C-y>", '"+y')
setk(n, "<C-a>", "gg^vG$<CR>")
setk(ni, "<C-s>", "<Esc>:w<CR>")
setk(ni, "<C-x>", "<Esc>:w<CR>:so<CR>")
setk(n, "°&", "xi")
setk(t, "<Esc>", [[<C-\><C-n>]])

setk(
	n,
	"<C-n>",
	":enew | setlocal nonu nornu nomodifiable<CR>",
	"Open empty immuable buffer"
)

setk(n, "è ", ":noh<CR>")
setk(n, ",rx", function()
	vim.cmd("write")
	os.execute("pkill -USR1 -x sxhkd")
end, "Reload sxhkd")
setk(n, ",i", ":NvimWebDeviconsHiTest<CR>")

---------------
--- Windows ---
---------------

local setwink = function(letter, command)
	local keymap = "<A-" .. letter .. ">"
	setk(nit, keymap, "<C-\\><C-n>" .. command)
	keymap = "<D-" .. letter .. ">"
	setk(n, keymap, command)
	setk(it, keymap, "<C-\\><C-n>" .. command, nil, 1)
end

setwink("h", "<C-w>h")
setwink("j", "<C-w>j")
setwink("k", "<C-w>k")
setwink("l", "<C-w>l")
setwink("g", ":vert res +20<CR>")
setwink("m", ":vert res -20<CR>")
setwink("ù", ":vert res -1<CR>")
setwink("f", ":vert res +1<CR>")
setwink("u", ":res +20<CR>")
setwink("n", ":res -20<CR>")
setwink("b", ":res -1<CR>")
setwink("y", ":res +1<CR>")

setk(n, "<C-*>", ":vsplit | term<CR>")
setk(n, "<C-ù>", ":split | term<CR>")

----------------
--- Markdown ---
----------------

local mdprefix = "ç"
local md = {
	i = "ysiw*",
	I = "ds*",
	u = "ysiw_",
	U = "ds_",
}

for k, v in pairs(md) do
	keymaps[k] = { description = mdprefix .. v, modes = { "n" } }
end

setk(n, "ç", function()
	local key = vim.fn.getcharstr()
	local cmd = md[key]
	if cmd then
		local termcode = api.nvim_replace_termcodes(cmd, true, false, true)
		api.nvim_feedkeys(termcode, "m", false)
	else
		api.nvim_feedkeys("ç" .. key, "n", false)
	end
end, nil, 1)

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

-----------
--- Git ---
-----------

setk(n, "gh", ":Gitsigns preview_hunk<CR>")
setk(n, "gi", ":Gitsigns preview_hunk_inline<CR>")
setk(n, "gb", ":Gitsigns toggle_current_line_blame<CR>")
setk(n, "gd", ":Gitsigns reset_hunk<CR>")
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
setk(n, "éz", ":Telescope frecency workspace=CWD<CR>")
setk(n, "éi", function()
	require("telescope.pickers")
		.new({}, {
			finder = require("telescope.finders").new_oneshot_job({
				"gh",
				"issue",
				"list",
				"--limit",
				"500",
			}),
			sorter = require("telescope.config").values.generic_sorter({}),
		})
		:find()
end, "Telescope list issues")
setk(n, "ép", function()
	require("telescope.pickers")
		.new({}, {
			finder = require("telescope.finders").new_oneshot_job({
				"gh",
				"pr",
				"list",
				"--limit",
				"500",
			}),
			sorter = require("telescope.config").values.generic_sorter({}),
		})
		:find()
end, "Telescope list prs")

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

setk(n, "àa", ":Trouble diagnostics<CR>")
setk(
	n,
	"àv",
	":Trouble diagnostics win.type=split win.position=right win.width=120<CR>"
)

setk(n, "àt", function()
	os.execute("cargo test -- --nocapture >output.txt 2>&1")
	vim.cmd("edit output.txt")
	vim.cmd("normal! G")
	vim.cmd([[syntax match CustomSuccess  /\<\(ok\|passed\)\>/]])
	vim.cmd([[syntax match CustomWarning  /\<\(ignored\|filtered out\)\>/]])
	vim.cmd(
		[[syntax match CustomError /\<\(failed\|FAILED\|error\|failures\)\>/]]
	)
end, "Open cargo tests report")

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
		if
			vim.lsp.get_client_by_id(client.id)
			and vim.lsp.buf_is_attached(buf, client.id)
		then
			print("Active LSP client:", client.name)
		end
	end
end, "Print active lsp clients")

--> meta programming <--

setk(n, "àc", [[:%s/.*#line.*\n//<CR>]])

------------
--- Case ---
------------

-- press ~ to toggle a letter between lower case and capitals
setk(nv, ",cc", ":TtCamel<CR>")
setk(nv, ",cp", ":TtPascal<CR>")
setk(nv, ",ck", ":TtKebab<CR>")
setk(nv, ",cs", ":TtSnake<CR>")
setk(nv, ",cn", ":TtConst<CR>")
setk(nv, ",cd", ":TtDot<CR>")
setk(nv, ",ct", ":TtTitle<CR>")

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

setk(n, ",r", function()
	local charset = {}
	for c = 48, 57 do
		table.insert(charset, string.char(c))
	end -- 0-9
	for c = 65, 90 do
		table.insert(charset, string.char(c))
	end -- A-Z
	for c = 97, 122 do
		table.insert(charset, string.char(c))
	end -- a-z

	math.randomseed(os.time() + math.floor(math.random() * 10000))
	local s = {}
	for j = 1, 10 do
		s[j] = charset[math.random(#charset)]
	end
	api.nvim_put({ table.concat(s) }, "c", true, true)
end, "Generate a random string")

---------------------
--- Custom helper ---
---------------------

local load_lines = function(width)
	local keys = {}
	for key, _ in pairs(keymaps) do
		table.insert(keys, key)
	end

	table.sort(keys)

	local lines = {
		"Custom keymaps (q to exit)",
		string.rep("-", width),
		"z=\tn\t Open spell suggestions",
		"]s\tn\t next spell error",
		"[s\tn\t previous spell error",
		"zg\tn\t mark word as correct",
	}

	for _, key in ipairs(keys) do
		local keymap = keymaps[key]
		local modes = table.concat(keymap.modes)
		local description = keymap.description or "nil"
		local line = key:gsub(" ", "␣")
			.. " \t"
			.. modes
			.. "\t "
			.. description
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

		api.nvim_buf_set_keymap(
			buf,
			"n",
			"q",
			":close<CR>",
			{ noremap = true, silent = true }
		)
	end,
})

print(nb)
