-- This file is sourced by the generate.lua script. It provides hard-coded
-- paths to other files needed in the generation process, in order to avoid
-- relying on external libraries like LuaFileSystem (portability concerns).

C_TEMPLATES = {
	dir = 'src/',

	'internal/internal.c',
	'queue/queue.c',
	'stack/stack.c',
	'utils/sp_utils.c',
}

H_TEMPLATES = {
	dir = 'src/',

	'internal.h',
	'sp_errcodes.h',
	'sp_queue.h',
	'sp_stack.h',
	'sp_utils.h',
	'staple.h',
}

MAN_TEMPLATES = {
	dir = 'man/',

	'libstaple.7',
	'libstaple_modes.3',
	'sp_free.3',
	{
		parent = 'man/sp_stack.7',
		dir = 'man/stack/',

		'sp_stack_clear.3',
		'sp_stack_copy.3',
		'sp_stack_create.3',
		'sp_stack_destroy.3',
		'sp_stack_foreach.3',
		'sp_stack_get.3',
		'sp_stack_insert.3',
		'sp_stack_peek.3',
		'sp_stack_pop.3',
		'sp_stack_print.3',
		'sp_stack_push.3',
		'sp_stack_qinsert.3',
		'sp_stack_qremove.3',
		'sp_stack_remove.3',
		'sp_stack_set.3',
	},
	{
		parent = 'man/sp_queue.7',
		dir = 'man/queue/',

		'sp_queue_clear.3',
		'sp_queue_copy.3',
		'sp_queue_create.3',
		'sp_queue_destroy.3',
		'sp_queue_foreach.3',
		'sp_queue_get.3',
		'sp_queue_insert.3',
		'sp_queue_peek.3',
		'sp_queue_pop.3',
		'sp_queue_print.3',
		'sp_queue_push.3',
		'sp_queue_qinsert.3',
		'sp_queue_qremove.3',
		'sp_queue_remove.3',
		'sp_queue_set.3',
	},
}

-- Iterator function for *_TEMPLATES tables
function templates_iter(templates)
	local flat = {}
	if templates.parent then
		flat[#flat + 1] = templates.parent
	end
	local prefix = templates.dir or ''
	for _, v in ipairs(templates) do
		if type(v) ~= 'table' then
			flat[#flat + 1] = prefix..v
		else
			for _, t in templates_iter(v) do
				flat[#flat + 1] = t
			end
		end
	end

	local i = 1
	return function()
		if i > #flat then
			return nil
		end
		i = i + 1
		return i-1, flat[i - 1]
	end
end

-- Used by the Makefile to get a complete list of filenames
function print_templates(templates)
	if templates == nil then
		print_templates(C_TEMPLATES)
		print_templates(H_TEMPLATES)
		print_templates(MAN_TEMPLATES)
		return
	end
	for _, t in templates_iter(templates) do
		print(t)
	end
end
