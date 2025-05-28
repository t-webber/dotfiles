local files = {
	"colours",
	"settings",
	"keybindings",
	"lsp",
	"lazyconfig",
}

for _, file in ipairs(files) do
	require(file)
end

--- TODO ---

-- Comment out code
-- LSP
-- Embedded terminal: support for vim bindings in terminal, colours, etc.
-- Better git
-- Clipboard history
-- Keybind list popup
