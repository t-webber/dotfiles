local files = {
	"statusline",
	"keymaps",
	"colours",
	"lsp",
	"settings",
	"functions",
	"lazyconfig",
}

for _, file in ipairs(files) do
	pcall(require, file)
end
for _, file in ipairs(files) do
	require(file)
end

vim.fn.writefile({}, vim.lsp.get_log_path())

--- TODO ---

-- Clipboard history
