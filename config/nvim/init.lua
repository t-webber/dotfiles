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

if os.getenv("DEVICE") == "v" then
	vim.g.python3_host_prog =
		"/arm/projectscratch/pd/pj1100048/oyl_venvs/tomweb01/1cfb64a465746095d450d9582a3bfeee2cf5226e1737afe4c888e382fe9c9892/x86_64_rhel8_python3.12.7/bin/python"
end

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
