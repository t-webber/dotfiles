local files = {
	"settings",
	"functions",
	"colours",
	"keymaps",
	"lsp",
	"lazyconfig",
}

for _, file in ipairs(files) do
	require(file)
end

--- TODO ---

-- Embedded terminal: support for vim bindings in terminal, colours, etc.
-- Clipboard history
