local api = vim.api
local fn = vim.fn
local keymap_display = {}
local keymap_taken = {}

local function describe(description, action, keymap)
	local t = type(description)
	if t == 'string' then
		return description
	end
	if description then
		error('unsupported description ' .. description)
	end
	if type(action) == 'string' then
		return action:gsub('<CR>', '󰌑 '):gsub('<Esc>', '󱊷 ')
	end
	error('misssing description for ' .. keymap)
end

local nb = 0

local function setk(modes, keymap, action, description, ignore)
	for _, mode in ipairs(modes) do
		vim.keymap.set(mode, keymap, action, {})
		nb = nb + 1
	end
	if ignore == nil then
		if keymap_taken[keymap] ~= nil then
			error('multiple keymaps for ' .. keymap)
		end
		keymap_taken[keymap] = 'taken'
		table.insert(keymap_display, {
			keymap = keymap,
			description = describe(description, action, keymap),
			modes = modes,
		})
	end
end

local n = { 'n' }
local ni = { 'n', 'i' }
local nv = { 'n', 'v' }
local nt = { 'n', 't' }
local niv = { 'n', 'i', 'v' }
local nit = { 'n', 'i', 't' }
local nvt = { 'n', 'v', 't' }
local nivt = { 'n', 'i', 'v', 't' }

local i = { 'i' }
local t = { 't' }
local it = { 'i', 't' }

local winicon = '🌄 '
local telescopeicon = '🔭 '
local giticon = '🦑 '
local oilicon = '🌻 '
local globalicon = '📢 '

---------------
--- General ---
---------------

setk(nivt, '<F1>', '<C-o>')
setk(nivt, '<F2>', '<C-i>')

setk(n, ',t', ':te<CR>')
setk(n, ',fr', function()
	os.execute('setxkbmap fr')
end, 'setxkbmap fr')
setk(n, ',o', function()
	if vim.fn.has('unix') == 1 then
		os.execute('xdg-open ' .. vim.fn.expand('<cWORD>'))
	else
		os.execute('open ' .. vim.fn.expand('<cWORD>'))
	end
end, 'open link')

setk(nivt, '<A-(>', '{')
setk(nivt, '<A-)>', '}')
setk(nivt, '<D-:>', '\\')
setk(i, '<D-(>', '[')
setk(i, '<D-)>', ']')
setk(i, '<D-é>', '~')

setk(nv, 'ça', 'gg^vG$<CR>')
setk(nv, 'çs', '<Esc>:w<CR>', 'save')
setk(niv, 'çx', '<Esc>:w<CR>:so<CR>', 'save and execute')
setk(niv, 'çd', function()
	vim.cmd('vnew | r # | normal! G<CR>')
	vim.cmd('diffthis')
	vim.cmd('wincmd h')
	vim.cmd('diffthis')
end, 'diff unsaved')

setk(nv, 'çe', function()
	vim.cmd('wincmd l')
	vim.cmd('q')
end, 'diff close')

setk(t, '<Esc>', [[<C-\><C-n>]])

setk(
	n,
	'<C-n>',
	':enew | setlocal nonu nornu nomodifiable<CR>',
	'Open empty immuable buffer'
)

setk(n, 'è ', ':noh<CR>')
setk(n, ',rx', function()
	vim.cmd('write')
	os.execute('pkill -USR1 -x sxhkd')
end, 'Reload sxhkd')

setk(n, ',i', ':NvimWebDeviconsHiTest<CR>')

setk(n, 'du', function()
	vim.cmd('normal! diwds(')
end, 'Delete fn name and ()')

------------------------
--- Global variables ---
------------------------

local function setglob(varname, value, reload)
	local var = require('globals')['statusline_' .. varname]
	vim.g[var] = value
	if reload then
		vim.cmd.redrawstatus()
	end
end

local function setglobswitch(letter, varname, value)
	setk(n, 'ùy' .. letter, function()
		setglob(varname, value, true)
	end, globalicon .. 'add ' .. varname)

	setk(n, 'ùd' .. letter, function()
		setglob(varname, '', true)
	end, globalicon .. 'del ' .. varname)
end

setglobswitch('t', 'treesitter', 'active')
setglobswitch('e', 'full_treesitter', 'active')
setglobswitch('l', 'linecol', 'active')
setglobswitch('f', 'filepath', 'file')
setglobswitch('d', 'filepath', 'dir')
setglobswitch('p', 'filepath', 'path')

setk(n, 'ùdw', function()
	setglob('notif', '')
	setglob('reg', '', true)
end, globalicon .. 'del regs and notif')

setk(n, 'ùys', function()
	vim.o.statusline = '%!v:lua.___custom_statusline_content()'
end, globalicon .. 'add sline')
setk(n, 'ùds', function()
	vim.o.statusline = ''
end, globalicon .. 'del sline')

-------------------------
--- Window navigation ---
-------------------------

local function setwink(letter, command)
	local keymap = '<A-' .. letter .. '>'
	setk(nit, keymap, '<C-\\><C-n>' .. command)
	keymap = '<D-' .. letter .. '>'
	setk(n, keymap, command)
	setk(it, keymap, '<C-\\><C-n>' .. command, winicon .. command, 'setwink')
end

setwink('h', '<C-w>h')
setwink('j', '<C-w>j')
setwink('k', '<C-w>k')
setwink('l', '<C-w>l')
setwink('g', ':vert res +20<CR>')
setwink('m', ':vert res -20<CR>')
setwink('ù', ':vert res -1<CR>')
setwink('f', ':vert res +1<CR>')
setwink('u', ':res +20<CR>')
setwink('n', ':res -20<CR>')
setwink('b', ':res -1<CR>')
setwink('y', ':res +1<CR>')

setk(n, '<C-*>', ':vsplit | term<CR>')
setk(n, '<C-ù>', ':split | term<CR>')

-------------------------
--- Window reordering ---
-------------------------

local function setw(letter, command, reverse)
	setk(nivt, '<A-' .. letter .. '>', function()
		local file = vim.fn.expand('%:f')
		vim.cmd('hide')
		if reverse then
			local cur = vim.api.nvim_get_current_win()
			vim.cmd(command)
			vim.api.nvim_set_current_win(cur)
		else
			vim.cmd(command)
		end
		vim.cmd('buf ' .. file)
	end, winicon .. 'switch to ' .. command .. (reverse and ' reverse' or ''))
end

setw('r', 'vsplit')
setw('R', 'vsplit', true)
setw('t', 'split')
setw('T', 'split', true)

-----------------------
--- Window exchange ---
-----------------------

local function sett(letter, direction)
	setk(nivt, '<A-' .. letter .. '>', function()
		local old_file = vim.fn.expand('%:f')
		local old_win = vim.api.nvim_get_current_win()
		vim.cmd('wincmd ' .. direction)
		local new_file = vim.fn.expand('%:f')
		local new_win = vim.api.nvim_get_current_win()
		if old_win == new_win then
			return
		end
		vim.cmd('buf ' .. old_file)
		vim.cmd('wincmd p')
		vim.cmd('buf ' .. new_file)
		vim.cmd('wincmd ' .. direction)
	end, winicon .. 'switch to ' .. direction)
end

sett('w', 'h')
sett('x', 'j')
sett('c', 'k')
sett('v', 'l')

----------------
--- Markdown ---
----------------

local mdprefix = 'ç'
local md = {
	i = 'ysiw*',
	I = 'ds*',
	u = 'ysiw_',
	U = 'ds_',
}

for k, v in pairs(md) do
	table.insert(
		keymap_display,
		{ keymap = mdprefix .. k, description = v, modes = { 'n' } }
	)
end

setk(n, 'ç', function()
	local key = vim.fn.getcharstr()
	local cmd = md[key]
	if cmd then
		local termcode = api.nvim_replace_termcodes(cmd, true, false, true)
		api.nvim_feedkeys(termcode, 'm', false)
	else
		api.nvim_feedkeys('ç' .. key, 'n', false)
	end
end, nil, 1)

-----------------
--- Iron REPL ---
-----------------

setk(n, 'è:', ':IronAttach<CR>')
setk(n, 'è!', ':IronHide<CR>')
setk(n, '<C-è>', function()
	local line = api.nvim_get_current_line()
	print(line)
	if line == '' then
		line = ' '
	end
	api.nvim_command('IronSend ' .. line:gsub(' ', '\\ '))

	api.nvim_command('normal! j')
end, 'IronSend')

-----------
--- Git ---
-----------

local function setg(letter, action)
	setk(n, 'g' .. letter, ':Gitsigns ' .. action .. '<CR>', giticon .. action)
end

setg('h', 'preview_hunk')
setg('i', 'preview_hunk_inline')
setg('t', 'toggle_current_line_blame')
setg('l', 'blame_line')
setg('b', 'blame')
setg('d', 'reset_hunk')

setk(n, 'ga', ':te git add -p<CR>', giticon .. 'add -p')
setk(n, 'go', ':Neogit<CR>', giticon .. 'neogit')

-----------------
--- Telescope ---
-----------------

local function setp(letter, cmd)
	setk(n, 'é' .. letter, function()
		vim.o.laststatus = 0
		vim.cmd('Telescope ' .. cmd)
		vim.o.laststatus = 2
	end, telescopeicon .. cmd)
end

setp('f', 'find_files')
setp('g', 'git_files')
setp('s', 'git_status')
setp('h', 'highlights')
setp('e', 'live_grep')
setp('r', 'registers')
setp('z', 'frecency workspace=CWD')

local function settg(letter, cmd, description, functions)
	setk(n, 'é' .. letter, function()
		vim.o.laststatus = 0
		local finder = require('telescope.finders').new_oneshot_job(
			cmd,
			{ entry_maker = functions and functions.entry_maker }
		)
		require('telescope.pickers')
			.new({}, {
				finder = finder,
				sorter = require('telescope.config').values.generic_sorter({}),
				attach_mappings = functions and functions.attach_mappings,
			})
			:find()
		vim.o.laststatus = 2
	end, description)
end

settg('i', {
	'gh',
	'issue',
	'list',
	'--limit',
	'500',
}, telescopeicon .. 'issues')

settg('p', {
	'gh',
	'pr',
	'list',
	'--limit',
	'500',
}, telescopeicon .. 'prs')

settg('n', {
	'fd',
	'--no-ignore',
	'--hidden',
	'-E target',
	'-E git',
	'-E node_modules',
	'-E venv',
}, telescopeicon .. 'hidden files')

settg(
	'x',
	{
		'clipcatctl',
		'list',
	},
	telescopeicon .. 'clipcat',
	{
		entry_maker = function(entry)
			local id, content = entry:match('^(.-):%s*(.*)$')
			local display = string.gsub(content, '\\n', ' | ')
			return { value = id, display = display, ordinal = content }
		end,
		attach_mappings = function(prompt_bufnr, map)
			local actions = require('telescope.actions')
			local action_state = require('telescope.actions.state')

			local function print_selection()
				local selected_entry = action_state.get_selected_entry()
				local selected_entry_id = selected_entry.value

				local clipboard_entry =
					vim.fn.system('clipcatctl get ' .. selected_entry_id)

				local clipboard_entry_fixed = string.gsub(
					string.gsub(clipboard_entry, '\\t', '\t'),
					'\\n',
					'\n'
				)
				vim.fn.setreg('+', clipboard_entry_fixed)
				actions.close(prompt_bufnr)
			end

			map('i', '<CR>', print_selection)
			map('n', '<CR>', print_selection)

			return true
		end,
	}
)

-----------
--- Oil ---
-----------

local icons = require('icons')

for _, folder in ipairs(icons) do
	setk(n, 'è' .. folder.short, function()
		require('oil').open(folder.var or '!')
	end, oilicon .. folder.icon)
end

setk(n, 'èk', ':Oil<CR>', oilicon .. '.')
setk(n, 'èj', function()
	local path = fn.expand('<cfile>')
	require('oil').open(path)
end, oilicon .. 'open file')

------------------
--- Formatting ---
------------------

local v = require('globals')
local pyfmt = v.pyfmt

setk(n, 'ùdy', function()
	vim.g[pyfmt] = ''
end, '🔧 Disable ruff')

setk(n, 'ùyy', function()
	vim.g[pyfmt] = 'active'
end, '🔧 Enable ruff')

setk(n, 'ùdo', function()
	require('conform').formatters_by_ft = {}
end, '🔧 Disable formatting')

setk(n, 'ùyo', function()
	require('conform').formatters_by_ft = require('fmt')
end, '🔧 Enable formatting')

-----------
--- LSP ---
-----------

--> errors <--

setk(n, 'àb', function()
	vim.lsp.buf.hover()
end, '💡 buf hover')
setk(n, 'àw', function()
	vim.diagnostic.open_float()
end, '💡 open float')

setk(n, 'àf', function()
	vim.lsp.buf.code_action()
end, '💡 code action')

setk(n, 'àa', ':Trouble diagnostics<CR>', '🚨 trouble diagnostics')
setk(
	n,
	'àv',
	':Trouble diagnostics win.type=split win.position=right win.width=120<CR>',
	'🚨 trouble diagnostics vert'
)

setk(n, 'àt', function()
	os.execute('cargo test -- --nocapture >output.txt 2>&1')
	vim.cmd('edit output.txt')
	vim.cmd('normal! G')
	vim.cmd([[syntax match CustomSuccess  /\<\(ok\|passed\)\>/]])
	vim.cmd([[syntax match CustomWarning  /\<\(ignored\|filtered out\)\>/]])
	vim.cmd(
		[[syntax match CustomError /\<\(failed\|FAILED\|error\|failures\)\>/]]
	)
end, '🧪 cargo test')

local function move_and_open(action)
	return function()
		vim.diagnostic.setqflist({ open = false })
		api.nvil_command(action)
		vim.defer_fn(function()
			vim.diagnostic.open_float(nil, { focus = false })
		end, 50)
	end
end

setk(n, 'àl', move_and_open('cnext'), 'cnext')
setk(n, 'àh', move_and_open('cprev'), 'cprev')
setk(n, 'àk', ':cclose<CR>', 'cclose')
setk(n, 'àj', ':copen<CR>', 'copen')

--> motion <--

setk(n, 'àd', function()
	vim.lsp.buf.definition({})
end, 'Go to definition')

--> lsp status <--

setk(n, 'às', ':checkhealth lsp<CR>')

setk(n, 'àp', function()
	local clients = vim.lsp.get_clients()
	local buf = api.nvim_get_current_buf()
	for _, client in ipairs(clients) do
		if
			vim.lsp.get_client_by_id(client.id)
			and vim.lsp.buf_is_attached(buf, client.id)
		then
			print('Active LSP client:', client.name)
		end
	end
end, 'Print active lsp clients')

--> meta programming <--

setk(n, 'àc', [[:%s/.*#line.*\n//<CR>]])

------------
--- Case ---
------------

-- press ~ to toggle a letter between lower case and capitals
setk(nv, ',cc', ':TtCamel<CR>', '🐪 camel')
setk(nv, ',cp', ':TtPascal<CR>', '🐪 pascal')
setk(nv, ',ck', ':TtKebab<CR>', '🐪 kebab')
setk(nv, ',cs', ':TtSnake<CR>', '🐪 snake')
setk(nv, ',cn', ':TtConst<CR>', '🐪 const')
setk(nv, ',cd', ':TtDot<CR>', '🐪 dot')
setk(nv, ',ct', ':TtTitle<CR>', '🐪 title')

-------------------------
--- Comment/Uncomment ---
-------------------------

setk(n, '<C-!>', function()
	local comment
	if vim.bo.filetype == 'python' then
		comment = '#'
	else
		local comments = vim.bo.comments
		comment = comments:match('([^:]+)$')
	end

	local pattern = '^%s*(' .. vim.pesc(comment) .. '+)%s*'

	local line = api.nvim_get_current_line()

	local _, _, match = line:find(pattern)

	if match then
		line = line:gsub(pattern, '')
	else
		line = comment .. ' ' .. line
	end

	api.nvim_set_current_line(line)
	vim.cmd('normal! j')
end, 'Comment/Uncomment line')

----------------------
--- Random strings ---
----------------------

setk(n, ',s', function()
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
	api.nvim_put({ table.concat(s) }, 'c', true, true)
end, 'Generate a random string')

---------------------
--- Custom helper ---
---------------------

local tsletters = require('tsletters')

local function load_lines(width)
	local lines = {
		'Keymaps (q to exit)',
		string.rep('-', width),
		'z=\tn\t Open spell suggestions',
		']s\tn\t next spell error',
		'[s\tn\t previous spell error',
		'zg\tn\t mark word as correct',
	}

	for _, keymap in ipairs(keymap_display) do
		local modes = table.concat(keymap.modes)
		local description = keymap.description or 'nil'
		local line = keymap.keymap:gsub(' ', '␣')
			.. ' \t'
			.. modes
			.. '\t '
			.. description
		table.insert(lines, line)
	end

	for letter, scope in pairs(tsletters.letters) do
		table.insert(lines, '[dvyc][s][ailr]' .. letter .. '\tnv\t' .. scope)
	end

	return lines
end

api.nvim_set_keymap('n', 'g?', '<Nop>', {
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
			style = 'minimal',
			relative = 'editor',
			width = width,
			height = height,
			row = row,
			col = col,
			border = 'rounded',
		}

		local win = api.nvim_open_win(buf, true, opts)
		api.nvim_win_set_option(win, 'winhl', 'Normal:Normal')

		api.nvim_buf_set_keymap(
			buf,
			'n',
			'q',
			':close<CR>',
			{ noremap = true, silent = true }
		)
	end,
})

print(nb)
