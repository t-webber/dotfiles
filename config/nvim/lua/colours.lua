local red = "#e06c75"
local green = "#98c379"
local cyan = "#77bbcc"
local grey = "#5c6350"
local yellow = "#e5c07b"
local magenta = "#c678dd"
local semidark = "#4b5263"
local orange = "#ff9955"
local darkblue = "#003366"
local darkgreen = "#114411"
local brown = "#442200"
local darkyellow = "#bbbb00"
local darkred = "#662222"
local lightred = "#f28a92"
local darkmagenta = "#660066"
local darkcyan = "#558899"
local dark = "#111109"
local white = "#ffffd0"
local black = "#000000"
local purple = "#553355"

local c = function(fg, bg, opts)
	return {
		fg = fg,
		bg = bg,
		bold = opts and opts.b,
		italic = opts and opts.i,
		strikethrough = opts and opts.s,
	}
end

local none = c("#ff0000")

local b = { b = true }
local i = { i = true }
local s = { s = true }

local title = c(magenta, black, b)

local colours = {
	--------------
	--- Window ---
	--------------
	CursorLineNr = c(red),
	CursorLine = c(nil, dark),
	StatusLine = c(nil, black),
	StatusLineNC = c(nil, black),
	Directory = c(red),
	TabLine = c(nil, black),
	TabLineSel = c(nil, dark),
	TabLineFill = c(nil, black),
	NormalFloat = c(magenta, black), -- registers and trouble windows
	WinSeparator = c(dark, dark), -- separator between buffers and borders around floating buffers

	--------------
	--- Editor ---
	--------------

	Normal = c(white, black),
	Search = c(nil, grey),
	IncSearch = c(yellow, darkred),
	CurSearch = c(black, lightred),

	--------------
	--- Syntax ---
	--------------
	Include = c(magenta),
	cIncluded = c(magenta),
	String = c(yellow),
	Folded = c(green),
	CustomSuccess = c(green),
	CustomIgnored = c(darkcyan),
	CustomWarning = c(yellow),
	CustomError = c(red),
	Title = c(magenta),
	Statement = c(red),
	JiraBold = c(green, nil, b),
	JiraItalic = c(cyan, nil, i),

	--> cf. https://neovim.io/doc/user/treesitter.html#treesitter-highlight-groups

	--- Vars ---
	["@variable"] = c(cyan), -- idents
	["@variable.builtin"] = c(darkcyan), -- this, self
	["@variable.parameter"] = c(cyan), -- params of functions, objects
	["@variable.parameter.builtin"] = c(darkcyan), -- special parameters (e.g. _, it, ..., this)
	["@variable.member"] = c(darkcyan), -- object and struct fields

	--- Consts ---
	["@constant"] = c(darkcyan), -- const idents
	["@constant.builtin"] = c(darkcyan), -- built-in constant values
	["@constant.macro"] = c(orange), -- constants defined by the preprocessor

	--- Mods ---
	["@module"] = c(orange), -- modules or namespaces
	["@module.builtin"] = c(red), -- built-in modules or namespaces

	--- Labels ---
	["@label"] = c(orange), -- C labels, Rust labels

	--- Strs ---
	["@string"] = c(yellow),
	["@string.documentation"] = c(grey), -- docstrings
	["@string.regexp"] = c(orange), -- regular expressions
	["@string.escape"] = c(orange), -- escape sequences
	["@string.special"] = c(orange), -- other special strings (e.g. dates)
	["@string.special.symbol"] = c(darkcyan), -- symbols or atoms (e.g., Makefile variables)
	["@string.special.path"] = c(yellow), -- filename, e.g. in git commit files
	["@string.special.url"] = none, -- URIs (e.g. hyperlinks)
	["@string.special.vimdoc"] = c(red), -- <thing> in vimdoc
	["@character"] = c(green), -- character literals
	["@character.special"] = c(orange), -- special characters (e.g. wildcards)

	--- Lits ---
	["@boolean"] = c(orange), -- boolean literals
	["@number"] = c(magenta), -- numeric literals
	["@number.float"] = c(magenta), -- floating-point number literals

	--- Types ---
	["@type"] = c(darkcyan), -- type or class definitions and annotations
	["@type.builtin"] = c(darkcyan), -- built-in types
	["@type.definition"] = c(darkcyan), -- identifiers in type definitions (e.g. typedef <type> <identifier> in C)
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
	["@markup.strong"] = c(orange, nil, b), -- bold text
	["@markup.italic"] = c(cyan, nil, i), -- italic text
	["@markup.strikethrough"] = c(yellow, nil, s), -- struck-through text
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
	["@tag.attribute"] = c(darkcyan), -- XML-style tag attributes
	["@tag.delimiter"] = c(magenta), -- XML-style tag delimiters

	--- LSP ---
	["@lsp.type.enumMember.rust"] = c(cyan),

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

	--------------
	--- Neogit ---
	--------------

	--> Main page <--

	NeogitSectionHeader = c(red),
	NeogitRemote = c(cyan),
	NeogitBranch = c(orange),
	NeogitBranchHead = c(orange),
	NeogitCommitViewHeader = title,

	--> Hunk preview page <--

	NeogitDiffAdd = c(nil, "#002200"),
	NeogitDiffDelete = c(nil, "#220000"),
	NeogitDiffAddHighlight = c(nil, "#003300"),
	NeogitDiffDeleteHighlight = c(nil, "#330000"),
	NeogitDiffAddCursor = c(nil, "#004400"),
	NeogitDiffDeleteCursor = c(nil, "#440000"),
	NeogitDiffContextHighlight = c(nil, black),
	NeogitDiffContextCursor = c(nil, dark),
	NeogitHunkHeader = title,
	NeogitHunkHeaderCursor = title,
	NeogitHunkHeaderHighlight = title,
	NeogitHunkMergeHeader = title,
	NeogitHunkMergeHeaderCursor = title,
	NeogitHunkMergeHeaderHighlight = title,
	NeogitDiffAdditions = c(green),
	NeogitDiffDeletions = c(red),
	NeogitFilePath = c(orange),
	NeogitDiffHeader = c(red, black, b),
	NeogitDiffHeaderHighlight = c(red, black, b),

	--> Graph <--

	NeogitObjectId = c(green),
	NeogitChangeModified = c(yellow),
	NeogitChangeNewFile = c(green),
	NeogitChangeRenamed = c(magenta),
	NeogitChangeDeleted = c(red),
	NeogitActiveItem = c(magenta, dark),
	NeogitGraphBoldBlue = c(orange),

	-------------------
	--- Status Line ---
	-------------------

	CustomStatusLineDefault = c(white),
	CustomStatusLineRegister = c(orange),
	CustomStatusLineHardtime = c(magenta),
	CustomStatusLineGood = c(green),
	CustomStatusLineBad = c(red),
	CustomStatusLineUnkown = none,

	--> Modes <--
	CustomStatusLineNormal = c(white, darkblue),
	CustomStatusLineInsert = c(white, darkgreen),
	CustomStatusLineSelect = c(white, purple),
	CustomStatusLineSelectLine = c(white, purple),
	CustomStatusLineSelectBlock = c(white, purple),
	CustomStatusLineVisual = c(white, darkred),
	CustomStatusLineVisualLine = c(white, darkred),
	CustomStatusLineVisualBlock = c(white, darkred),
	CustomStatusLineCommand = c(white, darkmagenta),
	CustomStatusLineReplace = c(white, darkcyan),
	CustomStatusLineTerminal = c(white, brown),

	--> Symbols <--
	CustomStatusLineNormalSymbol = c(cyan),
	CustomStatusLineInsertSymbol = c(green),
	CustomStatusLineSelectSymbol = c(magenta),
	CustomStatusLineSelectLineSymbol = c(magenta),
	CustomStatusLineSelectBlockSymbol = c(magenta),
	CustomStatusLineVisualSymbol = c(red),
	CustomStatusLineVisualLineSymbol = c(red),
	CustomStatusLineVisualBlockSymbol = c(red),
	CustomStatusLineCommandSymbol = c(magenta),
	CustomStatusLineReplaceSymbol = c(cyan),
	CustomStatusLineTerminalSymbol = c(yellow),

	----------------
	--- DevIcons ---
	----------------

	DevIconSh = c(green),
	DevIconLua = c(cyan),
	DevIconRust = c(orange),
	DevIconTypeScript = c(darkcyan),
	DevIconTsx = c(darkcyan),
	DevIconPrisma = c(magenta),
}

for name, options in pairs(colours) do
	vim.api.nvim_set_hl(0, name, options)
end
