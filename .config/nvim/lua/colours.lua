local red = "#e06c75"
local green = "#98c379"
local blue = "#60a0ff"
local cyan = "#77bbcc"
local grey = "#5c6370"
local yellow = "#e5c07b"
local magenta = "#c678dd"
local semidark = "#4b5263"
local orange = "#ff9955"
local darkblue = "#003366"
local darkgreen = "#114411"
local brown = "#442200"
local darkyellow = "#bbbb00"
local darkred = "#662222"
local darkmagenta = "#660066"
local darkcyan = "#558899"
local dark = "#111111"
local white = "#aaaaaa"
local black = "#000000"
local purple = "#553355"

local c = function(fg, bg)
	return { fg = fg, bg = bg }
end

local none = c("#ff0000")

local title = { fg = magenta, bold = true }

local colours = {
	--------------
	--- Window ---
	--------------
	Normal = c("#aaaaaa", "#000000"),
	CursorLineNr = c(red),
	CursorLine = c(nil, dark),
	StatusLine = c(nil, dark),
	Directory = c(red),

	--------------
	--- Syntax ---
	--------------
	Include = c(magenta),
	cIncluded = c(magenta),

	--> cf. https://neovim.io/doc/user/treesitter.html#treesitter-highlight-groups

	--- Vars ---
	["@variable"] = c(cyan), -- idents
	["@variable.builtin"] = c(darkcyan), -- this, self
	["@variable.parameter"] = c(cyan), -- params of functions, objects
	["@variable.parameter.builtin"] = none, -- special parameters (e.g. _, it)
	["@variable.member"] = c(darkcyan), -- object and struct fields

	--- Consts ---
	["@constant"] = c(darkcyan), -- const idents
	["@constant.builtin"] = c(darkcyan), -- built-in constant values
	["@constant.macro"] = c(orange), -- constants defined by the preprocessor

	--- Mods ---
	["@module"] = c(orange), -- modules or namespaces
	["@module.builtin"] = none, -- built-in modules or namespaces

	--- Labels ---
	["@label"] = c(orange), -- C labels, Rust labels

	--- Strs ---
	["@string"] = c(yellow),
	["@string.documentation"] = c(grey), -- docstrings
	["@string.regexp"] = none, -- regular expressions
	["@string.escape"] = c(orange), -- escape sequences
	["@string.special"] = none, -- other special strings (e.g. dates)
	["@string.special.symbol"] = c(blue), -- symbols or atoms (e.g., Makefile variables)
	["@string.special.path"] = c(yellow), -- filename, e.g. in git commit files
	["@string.special.url"] = none, -- URIs (e.g. hyperlinks)
	["@character"] = c(orange), -- character literals
	["@character.special"] = c(orange), -- special characters (e.g. wildcards)

	--- Lits ---
	["@boolean"] = c(orange), -- boolean literals
	["@number"] = c(magenta), -- numeric literals
	["@number.float"] = c(magenta), -- floating-point number literals

	--- Types ---
	["@type"] = c(blue), -- type or class definitions and annotations
	["@type.builtin"] = c(blue), -- built-in types
	["@type.definition"] = c(blue), -- identifiers in type definitions (e.g. typedef <type> <identifier> in C)
	["@attribute"] = c(magenta), -- attribute annotations (e.g. Python decorators, Rust lifetimes)
	["@attribute.builtin"] = c(magenta), -- builtin annotations (e.g. @property in Python)
	["@property"] = c(cyan), -- the key in key/value pairs

	--- Fns ---
	["@function"] = c(green), -- function definitions
	["@function.builtin"] = c(green), -- built-in functions
	["@function.call"] = c(green), -- function calls
	["@function.macro"] = c(magenta), -- C: macro def, Rust: macro call
	["@function.method"] = c(green), -- method definitions
	["@function.method.call"] = c(green), -- method calls, Python but not Rust
	["@constructor"] = c(red), -- constructor calls and definitions
	["@operator"] = c(magenta), -- symbolic operators (e.g. +, *)

	--- Keywds ---
	["@keyword"] = c(red), -- keywords not fitting into specific categories
	["@keyword.coroutine"] = c(red), -- keywords related to coroutines (e.g. go in Go, async/await in Python)
	["@keyword.function"] = c(red), -- keywords that define a function (e.g. func in Go, def in Python)
	["@keyword.operator"] = c(red), -- operators that are English words (e.g. and, or)
	["@keyword.import"] = c(magenta), -- keywords for including or exporting modules (e.g. import, from in Python)
	["@keyword.type"] = c(red), -- keywords describing namespaces and composite types (e.g. struct, enum)
	["@keyword.modifier"] = c(red), -- keywords modifying none constructs (e.g. const, static, public)
	["@keyword.repeat"] = c(red), -- keywords related to loops (e.g. for, while)
	["@keyword.return"] = c(red), -- keywords like return and yield
	["@keyword.debug"] = c(red), -- keywords related to debugging
	["@keyword.exception"] = c(red), -- keywords related to exceptions (e.g. throw, catch)
	["@keyword.conditional"] = c(red), -- keywords related to conditionals (e.g. if, else)
	["@keyword.conditional.ternary"] = c(red), -- ternary operator (e.g. ?, :)
	["@keyword.directive"] = c(red), -- various preprocessor directives and shebangs
	["@keyword.directive.define"] = c(red), -- preprocessor definition directives

	--- Puncs ---
	["@punctuation.delimiter"] = c(magenta), -- delimiters (e.g. ;, ., ,)
	["@punctuation.bracket"] = c(magenta), -- brackets (e.g. (), {}, [])
	["@punctuation.special"] = c(magenta), -- special symbols (e.g. {} in string interpolation)

	--- Comments ---
	["@comment"] = c(grey), -- line and block comments
	["@comment.documentation"] = c(grey), -- comments documenting code
	["@comment.error"] = c(red), -- error-type comments (e.g. ERROR, FIXME, DEPRECATED)
	["@comment.warning"] = c(darkyellow), -- warning-type comments (e.g. WARNING, FIX, HACK)
	["@comment.todo"] = c(darkblue), -- todo-type comments (e.g. TODO, WIP)
	["@comment.note"] = c(green), -- note-type comments (e.g. NOTE, INFO, XXX)

	--- Markup ---
	-- ["@markup.strong"] = { bold = true }, -- bold text
	-- ["@markup.italic"] = { italic = true }, -- italic text
	-- ["@markup.strikethrough"] = none, -- struck-through text
	-- ["@markup.underline"] = none, -- underlined text (only for literal underline markup!)
	["@markup.heading"] = title, -- headings, titles (including markers)
	-- ["@markup.heading.1"] = title, -- top-level heading
	--["@markup.heading.2"] = none, -- section heading
	--["@markup.heading.3"] = none, -- subsection heading
	--["@markup.heading.4"] = none, -- and so on
	--["@markup.heading.5"] = none, -- and so forth
	--["@markup.heading.6"] = none, -- six levels ought to be enough for anybody
	["@markup.quote"] = c(green), -- block quotes
	-- ["@markup.math"] = none, -- math environments (e.g. $ ... $ in LaTeX)
	["@markup.link"] = c(orange), -- text references, footnotes, citations, etc.
	["@markup.link.label"] = c(yellow), -- link, reference descriptions
	["@markup.link.url"] = c(cyan), -- URL-style links
	--["@markup.raw"] = none, -- literal or verbatim text (e.g. inline code)
	--["@markup.raw.block"] = none, -- literal or verbatim text as a stand-alone block
	["@markup.list"] = title, -- list markers
	--["@markup.list.checked"] = none, -- checked todo-style list markers
	--["@markup.list.unchecked"] = none, -- unchecked todo-style list markers

	--- Diff ---
	["@diff.plus"] = c(green), -- added text (for diff files)
	["@diff.minus"] = c(red), -- deleted text (for diff files)
	["@diff.delta"] = c(yellow), -- changed text (for diff files)

	--- Tags ---
	["@tag"] = c(cyan), -- XML-style tag names (e.g. in XML, HTML, etc.)
	["@tag.builtin"] = c(cyan), -- builtin tag names (e.g. HTML5 tags)
	["@tag.attribute"] = c(blue), -- XML-style tag attributes
	["@tag.delimiter"] = c(magenta), -- XML-style tag delimiters

	------------------
	---- Git Signs ---
	------------------
	GitSignsAdd = c("#66cc66"),
	GitSignsChange = c("#cccc66"),
	GitSignsDelete = c("#cc6666"),

	-----------
	--- Oil ---
	-----------
	OilFile = c(green),
	OilLink = c(magenta),
	OilLinkTarget = c(yellow),

	----------------------
	--- Oil Git Status ---
	----------------------
	OilGitStatusIndexUnmodified = c(grey),
	OilGitStatusWorkingTreeUnmodified = c(grey),
	OilGitStatusIndexIgnored = c(semidark),
	OilGitStatusWorkingTreeIgnored = c(semidark),
	OilGitStatusIndexUntracked = c(darkblue),
	OilGitStatusWorkingTreeUntracked = c(darkblue),
	OilGitStatusIndexAdded = c(green),
	OilGitStatusWorkingTreeAdded = c(green),
	OilGitStatusIndexCopied = c(cyan),
	OilGitStatusWorkingTreeCopied = c(cyan),
	OilGitStatusIndexDeleted = c(red),
	OilGitStatusWorkingTreeDeleted = c(red),
	OilGitStatusIndexModified = c(yellow),
	OilGitStatusWorkingTreeModified = c(yellow),
	OilGitStatusIndexRenamed = c(cyan),
	OilGitStatusWorkingTreeRenamed = c(cyan),
	OilGitStatusIndexTypeChanged = none,
	OilGitStatusWorkingTreeTypeChanged = none,
	OilGitStatusIndexUnmerged = c(darkred),
	OilGitStatusWorkingTreeUnmerged = c(darkred),
}

for name, options in pairs(colours) do
	vim.api.nvim_set_hl(0, name, options)
end

return {
	Foreground = white,
	Dark = dark,
	Background = black,
	Normal = darkblue,
	Insert = darkgreen,
	Select = purple,
	SelectLine = purple,
	SelectBlock = purple,
	Visual = darkred,
	VisualLine = darkred,
	VisualBlock = darkred,
	Command = darkmagenta,
	Replace = darkcyan,
	Terminal = brown,
}
