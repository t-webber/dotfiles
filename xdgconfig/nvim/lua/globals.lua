local uniq_prefix = 'custom_global_vars_'

local globals = {}

local vars = {
	'pyfmt',
	'statusline_notif',
	'statusline_reg',
	'statusline_linecol',
	'statusline_filepath',
	'statusline_treesitter',
	'statusline_full_treesitter',
	'statusline_battery',
	'statusline_time',
}

for _, var in ipairs(vars) do
	local real_var_name = uniq_prefix .. var
	globals[var] = real_var_name
	vim.g[real_var_name] = ''
end

vim.g[globals['statusline_filepath']] = 'dir' -- file, dir, path
vim.g[globals['statusline_treesitter']] = 'active'
vim.g[globals['statusline_linecol']] = 'active'
vim.g[globals['statusline_battery']] = 'active'
vim.g[globals['statusline_time']] = 'active'

return globals
