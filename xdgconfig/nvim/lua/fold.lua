function CustomFoldText()
	local start = vim.v.foldstart
	local count = vim.v.foldend - start + 1
	local line = vim.api.nvim_buf_get_lines(0, start - 1, start, false)[1]

	-- get treesitter highlights for the first line
	local chunks = {}
	for i = 0, #line - 1 do
		local captures = vim.treesitter.get_captures_at_pos(0, start - 1, i)
		local hl = captures[#captures] and ('@' .. captures[#captures].capture)
			or 'Folded'
		if #chunks > 0 and chunks[#chunks][2] == hl then
			chunks[#chunks][1] = chunks[#chunks][1] .. line:sub(i + 1, i + 1)
		else
			table.insert(chunks, { line:sub(i + 1, i + 1), hl })
		end
	end

	table.insert(chunks, { ' (' .. count .. ' lines) ', '@comment' })
	table.insert(chunks, { string.rep('─', 500), '@comment' })

	return chunks
end

vim.o.foldtext = 'v:lua.CustomFoldText()'
vim.o.fillchars = ''
vim.o.foldnestmax = 1
