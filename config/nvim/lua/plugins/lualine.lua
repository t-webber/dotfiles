local c = require("colours")
local fg = c.Foreground
local dark = { fg = fg, bg = c.dark }

local register = {
	function()
		local reg = vim.fn.reg_recording()
		if reg == "" then
			return ""
		end
		return "recording @" .. reg
	end,
	color = dark,
}

local mode = {
	"mode",
	color = function()
		local mode_colors = {
			n = c.Normal,
			i = c.Insert,
			v = c.Visual,
			V = c.VisualLine,
			["\22"] = c.VisualBlock,
			["\19"] = c.SelectBlock,
			s = c.Select,
			S = c.SelectLine,
			c = c.Command,
			R = c.Replace,
			t = c.Terminal,
		}
		local current_mode = vim.fn.mode()
		return { fg = "#aaaaaa", bg = mode_colors[current_mode] or "#ff0000" }
	end,
}

local battery = {
	function()
		local file = io.open("/sys/class/power_supply/BAT1/capacity", "r")
		if file then
			local content = file:read("*l")
			file:close()
			return string.format("%s%%%%", content)
		else
			return "!"
		end
	end,
	color = dark,
}

local position = {
	function()
		local line = vim.fn.line(".")
		local col = vim.fn.col(".")
		return string.format("%d:%d", line, col)
	end,
	color = dark,
}

local time = {
	function()
		return os.date("%H:%M")
	end,
	color = dark,
}

local sections = {
	lualine_a = { mode },
	lualine_b = {
		time,
		{ "filename", color = dark },
		register,
		{
			function()
				return vim.g.lualine_notification or "!"
			end,
			color = dark,
		},
	},
	lualine_c = {},
	lualine_x = {},
	lualine_y = { position, { "filetype", icon_only = false, colored = true, color = dark }, battery },
	lualine_z = {},
}

return {
	"nvim-lualine/lualine.nvim",
	config = function()
		require("lualine").setup({
			options = {
				theme = "auto",
				component_separators = "|",
				section_separators = "",
			},
			sections = sections,
		})
	end,
}
