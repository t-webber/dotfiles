local tscols = {
	['@function'] = {
		function_definition = true, -- python
		function_declaration = true, -- lua
		function_call = true, -- lua
		function_item = true, -- rust
		command = true, -- sh
		template_function = true, -- c++
		template_method = true, -- c++
	},
	['@constructor'] = {
		class_definition = true, -- python
		trait_item = true, -- rust
		struct_specifier = true, -- c++
	},
	['@variable'] = {
		field = true, --lua
		parameters = true, --lua
		field_declaration = true, --rust
		variable_assignment = true, -- sh
		enum_variant = true, -- rust
		variable_list = true, -- lua
		keyword_argument = true, -- python
		qualified_identifier = true, -- c++
		field_pattern = true, --rust
		variable_reference = true, -- make
		assignment_statement = true, --lua
	},
	['@type'] = {
		struct_expression = true, --rust
		struct_item = true, --rust
		enum_item = true, -- rust
		type_item = true, -- rust
		type_binding = true, -- rust
		const_item = true, --rust
		alias_declaration = true, -- c++
		static_item = true, -- rust
		scoped_identifier = true, -- rust
	},
	['@module'] = {
		mod_item = true, -- rust
		import_statement = true, -- python
		import_from_statement = true, -- python
		future_import_statement = true, -- python
		alias_declaration = true, --python
		namespace_alias_definition = true, -- c++
		extern_crate_declaration = true, -- rust
	},
	['@keyword'] = {
		preproc_ifdef = true, -- c++
		preproc_def = true, -- c++
		return_statement = 'ret', -- lua
		if_statement = 'if', -- lua
		elseif_statement = 'elif', -- lua
		else_statement = 'else', -- lua
		for_statement = 'for', -- lua
		while_statement = 'while', -- lua
		impl_item = 'impl', -- rust
		await_expression = 'await', -- rust
		lambda = 'lambda', -- python
		else_clause = 'else', -- python
		with_statement = 'with', -- python
		if_expression = 'if', --rust
		for_expression = 'for', --rust
		loop_expression = 'loop', --rust
		async_block = 'async', --rust
	},
	['@operator'] = {
		assign_expression = '=', -- lua
		parenthesized_expression = '()', -- lua
		bracket_index_expression = '[]', -- lua
		table_constructor = '[]=', -- lua
		reference_expression = '&', -- rust
		try_expression = '?', --rust
		attribute_item = '#[]', -- rust
	},
	['@keyword.import'] = {
		use_declaration = 'use',
	},
	['@comment'] = {
		comment = '#', -- lua
		line_comment = '#',
	},
	['@string'] = { string = '"' },
	hiddennodes = {},
}

local hiddennodes = {
	'chunk',
	'block',
	-- 	'expression_list',
	-- 	'comment_content',
	-- 	'string_content',
	-- 	'binary_expression',
	-- 	'binary_operator',
	-- 	'unary_expression',
	-- 	'for_generic_clause',
	-- 	'identifier',
	-- 	'variable_declaration',
	-- 	'dot_index_expression',
	-- 	'method_index_expression',
	-- 	'escape_sequence',
	-- 	'arguments',
	-- 	'source_file',
	-- 	'visibility_modifier',
	-- 	'field_declaration_list',
	-- 	'generic_type',
	-- 	'type_identifier',
	-- 	'declaration_list',
	-- 	'doc_comment',
	-- 	'outer_doc_comment_marker',
	-- 	'let_declaration',
	-- 	'call_expression',
	-- 	'field_expression',
	-- 	'field_identifier',
	-- 	'field_initializer_list',
	-- 	'field_initializer',
	-- 	'parameter',
	-- 	'expression_statement',
	-- 	'match_expression',
	-- 	'match_block',
	-- 	'match_arm',
	-- 	'match_pattern',
	-- 	'tuple_struct_pattern',
	-- 	'self',
	-- 	'enum_variant_list',
	-- 	'attribute',
	-- 	'ordered_field_declaration_list',
	-- 	'program',
	-- 	'word',
	-- 	'module',
	-- 	'assignment',
	-- 	'call',
	-- 	'argument_list',
	-- 	'boolean_operator',
	-- 	'string_start',
	-- 	'string_end',
	-- 	'integer',
	-- 	'text',
	-- 	'word',
	-- 	'makefile',
	-- 	'shell_text',
	-- 	'recipy_line',
	-- 	'true',
	-- 	'command_name',
	-- 	'primitive_type',
	-- 	'tuple_expression',
	-- 	'string_literal',
	-- 	'type_arguments',
	-- 	'self_parameter',
	-- 	'integer_literal',
	-- 	'range_expression',
	-- 	'compound_statement',
	-- 	'translation_unit',
}

for _, node in ipairs(hiddennodes) do
	tscols['hiddennodes'][node] = true
end

return tscols
