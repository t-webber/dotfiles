-------------------------
--- Lualine functions ---
-------------------------

vim.api.nvim_clear_autocmds({})

local v = require('globals')
local statusline_notif = v.statusline_notif
local statusline_reg = v.statusline_reg

vim.g[statusline_notif] = ''
local core_notify = vim.notify
vim.notify = function(msg, ...)
	if type(msg) ~= 'string' then error('found non string notification') end
	local key = msg:match('^You pressed the (.+) key too soon!')
	if key then
		vim.g[statusline_notif] = 'No ' .. key
		vim.cmd('redrawstatus')
		return
	end
	key = msg:match('The (.+) key is disabled!')
	if key then
		if key == '<Up>' then
			key = ''
		elseif key == '<Left>' then
			key = ''
		elseif key == '<Right>' then
			key = ''
		elseif key == '<Down>' then
			key = ''
		end
		vim.g[statusline_notif] = 'No ' .. key .. ' '
		vim.cmd('redrawstatus')
		return
	end
	local first, second = msg:match('Use (.+) instead of (.+)')
	if first and second then
		vim.g[statusline_notif] = first .. '  ' .. second
		vim.cmd('redrawstatus')
		return
	end
	if msg == 'No code actions available' then
		vim.g[statusline_notif] = msg
		vim.cmd('redrawstatus')
		vim.defer_fn(function()
			if vim.g[statusline_notif] == msg then
				vim.g[statusline_notif] = ''
			end
		end, 3000)
		return
	end
	core_notify('core notif: ' .. msg, ...)
end

vim.api.nvim_create_autocmd({ 'RecordingEnter' }, {
	callback = function()
		local reg = vim.fn.reg_recording()
		if reg == '' then
			vim.g[statusline_reg] = ''
		else
			vim.g[statusline_reg] = reg
		end
	end,
})

vim.api.nvim_create_autocmd({ 'RecordingLeave' }, {
	callback = function() vim.g[statusline_reg] = '' end,
})

------------------------
--- Custom languages ---
------------------------

vim.filetype.add({
	extension = {
		todo = 'todo',
	},
})

local todo_group = vim.api.nvim_create_augroup('', { clear = true })

vim.api.nvim_create_autocmd({ 'BufRead', 'BufNewFile', 'BufWinEnter' }, {
	group = todo_group,
	pattern = '*.todo',
	callback = function()
		vim.fn.clearmatches()

		vim.fn.matchadd('Title', '^#.*$')

		-- Checkbox letter
		vim.fn.matchadd('TodoChecked', '^- \\[x\\].*$')
		vim.fn.matchadd('TodoPartial', '^- \\[\\.\\].*$')
		vim.fn.matchadd('TodoHold', '^- \\[-\\].*$')
		vim.fn.matchadd('TodoUnchecked', '^- \\[ \\].*$')

		local colours = require('colours')
		local true_colours = colours['true_colours']

		for name, value in pairs(true_colours) do
			vim.fn.matchadd(name, '^- \\[' .. value[2] .. '\\].*$')
		end

		-- End sequence
		for name, value in pairs(true_colours) do
			vim.fn.matchadd('bg' .. name, '/' .. value[2] .. '/ .*$')
		end

		vim.fn.matchadd('Hide', '/./ ')

		vim.keymap.set('n', '!', function()
			local l = vim.api.nvim_get_current_line()

			-- Toggle logic
			if l:match('^%- %[x%]') then
				-- Has x → remove
				l = l:gsub('^%- %[x%]', '- [ ]')
			elseif l:match('^%- %[%s%]') then
				-- Empty → add x
				l = l:gsub('^%- %[%s%]', '- [x]')
			end

			vim.api.nvim_set_current_line(l)
		end, { buffer = true, noremap = true, silent = true })
	end,
})

-----------------------
--- Other functions ---
-----------------------

local better_prisma_format = os.getenv('CMD') .. '/sh/paff'

vim.api.nvim_create_autocmd('BufWritePost', {
	pattern = '*.prisma',
	callback = function()
		local start = vim.loop.hrtime()

		os.execute(better_prisma_format)

		local paff = vim.loop.hrtime()

		local path = vim.api.nvim_buf_get_name(0)
		vim.cmd('bdelete!')
		vim.cmd('edit ' .. vim.fn.fnameescape(path))
		vim.cmd('filetype detect')

		local crazy_stuff = vim.loop.hrtime()

		local paff_time = math.floor((paff - start) / 1e6)
		local crazy_stuff_time = math.floor((crazy_stuff - paff) / 1e6)
		vim.g[statusline_notif] = paff_time .. ' & ' .. crazy_stuff_time
	end,
})

vim.api.nvim_create_autocmd('TextYankPost', {
	callback = function() vim.hl.on_yank() end,
})

vim.api.nvim_create_user_command('GitBlameLine', function()
	local line_number = vim.fn.line('.') -- Get the current line number. See `:h line()`
	local filename = vim.api.nvim_buf_get_name(0)
	print(
		vim.fn.system({ 'git', 'blame', '-L', line_number .. ',+1', filename })
	)
end, {})

vim.api.nvim_create_autocmd({ 'BufLeave', 'BufWinLeave' }, {
	callback = function(event)
		if vim.bo[event.buf].filetype == 'TelescopePrompt' then
			vim.o.laststatus = 2
		end
	end,
})

-- nvim 0.12 only
-- vim.api.nvim_create_autocmd('LspAttach', {
-- 	callback = function(args)
-- 		local client = vim.lsp.get_client_by_id(args.data.client_id)
--
-- 		if client:supports_method('textDocument/documentColor') then
-- 			vim.lsp.document_color.enable(false, args.buf)
-- 		end
-- 	end,
-- })
