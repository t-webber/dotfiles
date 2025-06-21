local globals = {
	lualine_notif = "___custom_lualine_notification_message",
	lualine_reg = "___custom_lualine_register_message",
}

for _, v in pairs(globals) do
	vim.g[v] = ""
end

return globals
