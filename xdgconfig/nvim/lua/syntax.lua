local colours = require('colours')
local true_colours = colours['true_colours']

---------------------
--- Global syntax ---
---------------------

function match(colour, pattern) vim.fn.matchadd(colour, pattern) end

local comment_char = { '-', '#', '/', '>', '<' }

local function apply_comment_colours()
	for _, ch in pairs(comment_char) do
		local ch2 = ch .. ch .. '*'
		local ch4 = ch .. ch .. ch .. ch .. '*'
		match('bg-hide-semidark', '^\\s*' .. ch4)
		match('bg-hide-semidark', ch4 .. '$')

		for name, value in pairs(true_colours) do
			if value[2] ~= 'b' then
				local pat = ch2 .. value[2] .. ch2
				local patt = ch2 .. value[2] .. '!' .. ch2
				local pattt = ch2 .. value[2] .. ':' .. ch2
				match(name, pat .. '.*$')
				match('bg-' .. name, patt .. '.*$')
				match('bg-' .. name, pattt .. '.*$')
				match('Hide', pat .. ' ')
				match('Hide', patt .. ' ')
				match('bg-hide-' .. name, pat)
				match('bg-hide-' .. name, patt)
				match('bg-hide-' .. name, pattt)
			end
		end
	end

	for _, seq in pairs({ '^<<<<<', '^>>>>>', '^=====' }) do
		match('bg-hide-truered', seq)
	end
end

local function y_all()
	vim.fn.clearmatches()
	apply_comment_colours()
end

local function toggle_checked()
	local l = vim.api.nvim_get_current_line()
	if l:match('^%- %[x%]') then
		l = l:gsub('^%- %[x%]', '- [ ]')
	elseif l:match('^%- %[%s%]') then
		l = l:gsub('^%- %[%s%]', '- [x]')
	end
	vim.api.nvim_set_current_line(l)
end

local function checkbox_letters()
	match('TodoChecked', '^- \\[x\\].*$')
	match('TodoPartial', '^- \\[\\.\\].*$')
	match('TodoHold', '^- \\[-\\].*$')
	match('TodoUnchecked', '^- \\[ \\].*$')

	for name, value in pairs(true_colours) do
		match(name, '^- \\[' .. value[2] .. '\\].*$')
	end
end

local function y_todo()
	vim.fn.clearmatches()

	checkbox_letters()
	apply_comment_colours()
	match('Title', '^#.*$')

	vim.keymap.set('n', '!', function() toggle_checked() end)
end

----------------------------
--- Apply y_* on BufRead ---
----------------------------

vim.filetype.add({
	extension = {
		todo = 'todo',
	},
})

local todo_group = vim.api.nvim_create_augroup('', { clear = true })

vim.api.nvim_create_autocmd({ 'BufRead', 'BufNewFile', 'BufWinEnter' }, {
	pattern = '!*.raw',
	callback = function() y_all() end,
})

vim.api.nvim_create_autocmd({ 'BufRead', 'BufNewFile', 'BufWinEnter' }, {
	group = todo_group,
	pattern = '*.todo',
	callback = function() y_todo() end,
})

y_all()
