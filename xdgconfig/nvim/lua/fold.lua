function CustomFoldText()
        local start = vim.v.foldstart
        local count = vim.v.foldend - start + 1
        local line = vim.api.nvim_buf_get_lines(0, start - 1, start, false)[1]

        -- get treesitter highlights for the first line
        local chunks = {}
        for i = 0, #line - 1 do
                local captures = vim.treesitter.get_captures_at_pos(0, start - 1, i)
                local hl = captures[#captures] and ('@' .. captures[#captures].capture) or 'Folded'
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

function CustomFoldExpr()
        local lnum = vim.v.lnum - 1

        -- Get the line text to find where the actual code starts
        local line = vim.fn.getline(vim.v.lnum)
        local col = line:find('%S') -- Find first non-whitespace character

        -- If it's a blank line, use the fold level of the previous line
        if not col then return '=' end

        -- Get the node at the start of the text (0-indexed column)
        local ok, node = pcall(vim.treesitter.get_node, { pos = { lnum, col - 1 } })
        if not ok or not node then return '0' end

        -- Walk up the tree to find a function-like parent
        local current = node
        while current do
                local type = current:type()
                if type:find('function') or type:find('method') or type == 'function_item' then
                        local start_row, _, end_row, _ = current:range()

                        if lnum == start_row then
                                return '>1' -- Start fold
                        elseif lnum < end_row then
                                return '1' -- Inside fold
                        end
                end
                current = current:parent()
        end

        return '0'
end
