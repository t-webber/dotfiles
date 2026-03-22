vim.fn.writefile({}, vim.lsp.log.get_filename())
vim.fn.writefile({}, os.getenv('XDG_STATE_HOME') .. '/nvim/conform.log')

local files = {
	'keymaps',
	'colours',
	'lsp',
	'settings',
	'functions',
	'statusline',
	'lazyconfig',
	'syntax',
}

for _, file in ipairs(files) do
	--pcall(require, file)
end
for _, file in ipairs(files) do
	require(file)
end

vim.api.nvim_create_autocmd('User', {
	pattern = 'VeryLazy',
	callback = function()
		package.loaded['colours'] = nil
		require('colours')
	end,
})
