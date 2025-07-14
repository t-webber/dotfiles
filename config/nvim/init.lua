local files = {
	"statusline",
	"keymaps",
	"colours",
	"lsp",
	"settings",
	"functions",
	"lazyconfig",
}

vim.fn.writefile({}, vim.lsp.get_log_path())
vim.fn.writefile({}, os.getenv("XDG_STATE_HOME") .. "/nvim/conform.log")

for _, file in ipairs(files) do
	pcall(require, file)
end
for _, file in ipairs(files) do
	require(file)
end

vim.api.nvim_create_autocmd("User", {
	pattern = "VeryLazy",
	callback = function()
		package.loaded["colours"] = nil
		require("colours")
	end,
})

--- TODO ---

-- Clipboard history
