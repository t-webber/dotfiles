local colours = require('colours')
local true_colours = colours['true_colours']

---------------------
--- Global syntax ---
---------------------

local comment_char = { '-', '#', '/', '>', '<' }

local function apply_comment_colours()
	for _, ch in pairs(comment_char) do
		local ch2 = ch .. ch .. '*'
		local ch4 = ch .. ch .. ch .. ch .. '*'
		vim.fn.matchadd('bg-hide-semidark', '^ *' .. ch4)
		vim.fn.matchadd('bg-hide-semidark', ch4 .. '$')

		for name, value in pairs(true_colours) do
			local pat = ch2 .. value[2] .. ch2
			local patt = ch2 .. value[2] .. '!' .. ch2
			local pattt = ch2 .. value[2] .. ':' .. ch2
			vim.fn.matchadd(name, pat .. '.*$')
			vim.fn.matchadd('bg-' .. name, patt .. '.*$')
			vim.fn.matchadd('bg-' .. name, pattt .. '.*$')
			vim.fn.matchadd('Hide', pat .. ' ')
			vim.fn.matchadd('Hide', patt .. ' ')
			vim.fn.matchadd('bg-hide-' .. name, pat)
			vim.fn.matchadd('bg-hide-' .. name, patt)
			vim.fn.matchadd('bg-hide-' .. name, pattt)
		end
	end

	for _, seq in pairs({ '^<<<<<', '^>>>>>', '^=====' }) do
		vim.fn.matchadd('bg-hide-truered', seq)
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
	vim.fn.matchadd('TodoChecked', '^- \\[x\\].*$')
	vim.fn.matchadd('TodoPartial', '^- \\[\\.\\].*$')
	vim.fn.matchadd('TodoHold', '^- \\[-\\].*$')
	vim.fn.matchadd('TodoUnchecked', '^- \\[ \\].*$')

	for name, value in pairs(true_colours) do
		vim.fn.matchadd(name, '^- \\[' .. value[2] .. '\\].*$')
	end
end

local function y_todo()
	vim.fn.clearmatches()

	checkbox_letters()
	apply_comment_colours()
	vim.fn.matchadd('Title', '^#.*$')

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
	callback = function() y_all() end,
})

vim.api.nvim_create_autocmd({ 'BufRead', 'BufNewFile', 'BufWinEnter' }, {
	group = todo_group,
	pattern = '*.todo',
	callback = function() y_todo() end,
})

y_all()
