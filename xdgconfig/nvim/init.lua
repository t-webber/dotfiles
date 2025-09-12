-- TODO: pipeline not working
-- TODO: inteligent lsp starting instead of buffer based

vim.fn.writefile({}, vim.lsp.log.get_filename())
vim.fn.writefile({}, os.getenv('XDG_STATE_HOME') .. '/nvim/conform.log')

local files = {
        'utils',
        'colours',
        'lsp',
        'fold',
        'settings',
        'statusline',
        'lazyconfig',
        'syntax',
        'functions',
        'autocmd',
        'keymaps',
}

local original_set = vim.diagnostic.set
vim.diagnostic.set = function(namespace, bufnr, diagnostics, opts)
        -- Check if the target buffer isn't loaded yet
        if bufnr and not vim.api.nvim_buf_is_loaded(bufnr) then
                -- Print the file that initiated this call
                local info = debug.getinfo(2, 'S')
                if info and info.source then
                        print('⚠️ SUSPECT CALLING DIAGNOSTICS ON UNLOADED BUF: ' .. info.source)
                end
        end
        return original_set(namespace, bufnr, diagnostics, opts)
end

for i, file in ipairs(files) do
        if i ~= #files then pcall(require, file) end
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
