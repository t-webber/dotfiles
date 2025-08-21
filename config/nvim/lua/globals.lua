local globals = {
	statusline_notif = "___custom_statusline_notification_message",
	statusline_reg = "___custom_statusline_register_message",
}

for _, v in pairs(globals) do
	vim.g[v] = ""
end

return globals
