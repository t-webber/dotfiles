local v = require('globals')
local statusline_treesitter = v.statusline_treesitter
local function make_hg(hg_name) return '%#' .. hg_name .. '#' end

---

local function nodext(child) return vim.treesitter.get_node_text(child, 0) end
local function tsvb() return vim.g[statusline_treesitter] end

local function ins(ns, vs, hg, vslvl)
        if hg == nil then hg = '@keyword' end
        if vslvl == nil then vslvl = 2 end
        if vs ~= nil and tsvb() >= vslvl then
                vs = string.gsub(vs, '%s+', ' ')
                if #vs >= 30 then vs = string.sub(vs, 1, 28) .. '..' end
                return make_hg(hg) .. ns .. '{' .. make_hg('Comment') .. vs .. make_hg(hg) .. '}'
        else
                return make_hg(hg) .. ns
        end
end

local function find_concat(node, names, sep)
        local txt = ''
        for child in node:iter_children() do
                if vim.tbl_contains(names, child:type()) then
                        if txt ~= '' and sep then txt = txt .. sep end
                        txt = txt .. nodext(child)
                end
        end
        return string.gsub(txt, '%s+', ' ')
end

local function ts_disp_fn_param(param)
        if param:type() == ',' then return ',' end
        if param:type() == 'identifier' then return nodext(param) end
        local prev_col = false
        local txt = ''
        for decl_item in param:iter_children() do
                if decl_item:type() == ':' then
                        prev_col = true
                elseif prev_col then
                        if tsvb() == 3 then
                                local y = nodext(decl_item)
                                y = string.gsub(y, '&mut ', '@')
                                y = string.gsub(y, ' ', '')
                                txt = txt .. ':' .. y
                        end
                        prev_col = false
                else
                        txt = txt .. nodext(decl_item)
                end
        end
        return txt
end

local function ts_disp_fn_generic(generic)
        local generics = {}
        for gen in generic:iter_children() do
                if gen:type() == 'type_parameter' then
                        for gen_item in gen:iter_children() do
                                if gen_item:type() == 'type_identifier' then
                                        table.insert(generics, nodext(gen_item))
                                elseif gen_item:type() == 'trait_bounds' and tsvb() == 3 then
                                        generics[#generics] = generics[#generics]
                                                .. ':'
                                                .. nodext(gen_item)
                                end
                        end
                end
        end
        if generics == {} then return '' end
        return '<' .. table.concat(generics, ',') .. '>'
end

local function ts_disp_fn(node, ty, ft)
        local fns = {
                lua = { 'function_declaration', 'function_definition' },
                py = { 'function_definition' },
                rs = { 'function_item' },
        }
        if not vim.tbl_contains(fns[ft] or {}, ty) then return end

        local fn = make_hg('@function')
        local comm = make_hg('Comment')

        local name = find_concat(node, { 'identifier' })
        if name == '' then name = make_hg('FunctionItalic') .. 'fn' .. fn end
        if tsvb() == 1 then return fn .. name end

        local generics = ''
        local params = ''
        local ret = ''

        local found_ret = false
        for fn_item in node:iter_children() do
                if fn_item:type() == 'type_parameters' then
                        generics = ts_disp_fn_generic(fn_item)
                elseif fn_item:type() == '->' then
                        found_ret = true
                elseif found_ret then
                        if tsvb() == 3 then ret = nodext(fn_item) end
                        break
                elseif fn_item:type() == 'parameters' then
                        for param in fn_item:iter_children() do
                                if not vim.tbl_contains({ '(', ')' }, param:type()) then
                                        params = params .. ts_disp_fn_param(param)
                                end
                        end
                end
        end

        params = '(' .. params .. ')'
        ret = string.gsub(ret, '%s+', '')

        local txt = fn
                .. T3(ret == '', '', '{')
                .. name
                .. comm
                .. generics
                .. params
                .. T3(ret == '', '', fn .. '  ' .. comm .. ret .. fn .. '}')

        txt = string.gsub(txt, '&mutself', '@self')
        txt = string.gsub(txt, ',%)', ')')

        return txt
end

local function ts_disp_var(node, ty, ft)
        if ft == 'lua' and ty == 'assignment_statement' then
                return make_hg('@variable') .. nodext(node:child(0):child(0)) .. '='
        end
        if ft == 'py' and ty == 'assignment' then
                return make_hg('@variable') .. nodext(node:child(0)) .. '='
        end
end

local function split_first(str, sep)
        local start_idx, end_idx = string.find(str, sep, 1, true)
        if not start_idx then return str, '' end
        return string.sub(str, 1, start_idx - 1), string.sub(str, end_idx + 1)
end

local function ts_disp_node(node, ft)
        -- sometimes positional because name can change, sometimes name because position can change
        local ty = node:type()
        if not ty then return '' end

        --- keywords ---
        local ones = {
                lua = { 'for_statement', 'if_statement', 'elseif_statement', 'while_statement' },
                rs = { 'match_expression', 'while_expression' },
                py = {
                        'match_statement',
                        'while_statement',
                        'with_statement',
                        'except_clause',
                        'if_statement',
                        'elif_clause',
                },
        }
        for _, val in ipairs(ones[ft] or {}) do
                if ty == val then
                        local name = string.match(val, '^([^_]*)')
                        return ins(name, nodext(node:child(1)))
                end
        end
        local nones = { py = { 'try_statement', 'finally_clause' }, rs = { 'loop_expression' } }
        for _, val in ipairs(nones[ft] or {}) do
                if ty == val then
                        local name = string.match(val, '^([^_]*)')
                        return make_hg('@keyword') .. name
                end
        end
        if ft == 'rs' and ty == 'for_expression' or ft == 'py' and ty == 'for_statement' then
                return ins('for', nodext(node:child(1)) .. ' in ' .. nodext(node:child(3)))
        end
        if ft == 'rs' then
                for _, x in ipairs({ 'struct', 'trait', 'enum' }) do
                        if ty == x .. '_item' then
                                local vb =
                                        find_concat(node, { 'type_identifier', 'type_parameters' })
                                return ins(x, vb)
                        end
                end

                if ty == 'impl_item' then
                        local tys = { 'type_identifier', 'generic_type' }
                        local txt = ins('impl', find_concat(node, tys, ' for '), '@keyword')
                        local first, second = split_first(txt, ' for ')
                        if second == nil or second == '' then return first end
                        return first .. make_hg('@keyword') .. '|' .. make_hg('Comment') .. second
                end
        end
        if ft == 'py' then
                if ty == 'class_definition' then
                        local tys = {}
                        if tsvb() >= 1 then table.insert(tys, 'identifier') end
                        if tsvb() >= 3 then table.insert(tys, 'argument_list') end
                        return ins('class', find_concat(node, tys), '@keyword')
                end
        end

        if ft == 'rs' then
                if ty == 'mod_item' then
                        return make_hg('@module') .. nodext(node:child(1)) .. '['
                end
                if ty == 'macro_definition' then
                        return make_hg('@module') .. nodext(node:child(1)) .. '!'
                end
        end

        if ft == 'lua' and ty == 'function_call' then
                return make_hg('@function') .. nodext(node:child(0)) .. '°'
        end
        if ft == 'py' and ty == 'call' then
                return make_hg('@function') .. nodext(node:child(0)) .. '°'
        end

        return ts_disp_var(node, ty, ft) or ts_disp_fn(node, ty, ft) or ''
end

local function main_one_node(node, ft)
        if tsvb() == 4 then
                local names = {}
                table.insert(names, node:type())
                for child in node:iter_children() do
                        table.insert(names, child:type())
                end
                return make_hg('none') .. string.sub(table.concat(names, '|'), 1, 100)
        end

        if ft == 'rust' then ft = 'rs' end
        if ft == 'python' then ft = 'py' end

        local txt = ts_disp_node(node, ft)
        txt = string.gsub(txt, '([^a-zA-Z0-9])self([^a-zA-Z0-9])', '%1󱐋%2')
        txt = string.gsub(txt, '([^a-zA-Z0-9])cls([^a-zA-Z0-9])', '%1%2')
        return txt
end

return function(ft, sep)
        local node = vim.treesitter.get_node()
        if tsvb() == 0 or not node then return '' end

        -- ast
        local path = ''
        while node do
                local new = main_one_node(node, ft)
                path = new .. path
                if new ~= '' then
                        path = sep .. path
                        if tsvb() == 4 then break end
                end
                node = node:parent()
        end

        return path
end
