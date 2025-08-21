local globals = {
	statusline_notif = "___custom_statusline_notification_message",
	statusline_reg = "___custom_statusline_register_message",
        pyfmt = "___custom_lsp_pyfmt_enabled",
}

for _, v in pairs(globals) do
	vim.g[v] = ""
end

return globals
