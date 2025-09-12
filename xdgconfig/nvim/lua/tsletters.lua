local letters = {
	q = '@assignment',
	t = '@attribute',
	b = '@block',
	k = '@call',
	c = '@class',
	h = '@comment',
	i = '@conditional',
	f = '@frame',
	o = '@function',
	l = '@loop',
	e = '@parameter',
	x = '@regex',
	r = '@return',
	s = '@statement',
}

local select = {
	lq = '@assignment.lhs',
	rq = '@assignment.rhs',
}

for letter, scope in pairs(letters) do
	select['a' .. letter] = scope .. '.outer'
	select['i' .. letter] = scope .. '.inner'
end

local goto_next_start = {}
local goto_prev_start = {}

for code, scope in pairs(select) do
	goto_next_start[')' .. code] = scope
	goto_prev_start['(' .. code] = scope
end

return {
	select = select,
	goto_next_start = goto_next_start,
	goto_prev_start = goto_prev_start,
	letters = letters,
}
