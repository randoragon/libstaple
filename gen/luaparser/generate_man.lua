--------------------------------------------------------------------------------
-- This file exposes the generate_man function used for generating man pages.
--------------------------------------------------------------------------------

require(DIRNAME..'luaparser.ParamConfig')
require(DIRNAME..'luaparser.Snippet')

-- Comparator function for sorting SEE ALSO. Note that this list comprises of
-- man page NAME section entries, not their physical file names.
local MAN_SORT_ORDER = {
	'libstaple(7)',
	'sp_stack(7)',
	'sp_queue(7)',

	'sp_stack_create(3)',
	'sp_stack_destroy(3)',
	'sp_stack_clear(3)',
	'sp_stack_push(3)',
	'sp_stack_peek(3)',
	'sp_stack_pop(3)',
	'sp_stack_insert(3)',
	'sp_stack_remove(3)',
	'sp_stack_qinsert(3)',
	'sp_stack_qremove(3)',
	'sp_stack_get(3)',
	'sp_stack_set(3)',
	'sp_stack_eq(3)',
	'sp_stack_copy(3)',
	'sp_stack_foreach(3)',
	'sp_stack_print(3)',

	'sp_queue_create(3)',
	'sp_queue_destroy(3)',
	'sp_queue_clear(3)',
	'sp_queue_push(3)',
	'sp_queue_peek(3)',
	'sp_queue_pop(3)',
	'sp_queue_insert(3)',
	'sp_queue_remove(3)',
	'sp_queue_qinsert(3)',
	'sp_queue_qremove(3)',
	'sp_queue_get(3)',
	'sp_queue_set(3)',
	'sp_queue_eq(3)',
	'sp_queue_copy(3)',
	'sp_queue_foreach(3)',
	'sp_queue_print(3)',

	'sp_free(3)',
	'sp_is_debug(3)',
	'sp_is_quiet(3)',
	'sp_is_abort(3)',

	'scanf(3)',
	'free(3)',
	'memcpy(3)',
	'memcmp(3)',
}
local sort_order = {}
for i, v in ipairs(MAN_SORT_ORDER) do
	sort_order[v] = i
end
local function see_also_comp(a, b)
	if sort_order[a] == nil then
		error(a..' not found in MAN_SORT_ORDER\n')
	end
	if sort_order[b] == nil then
		error(b..' not found in MAN_SORT_ORDER\n')
	end
	return sort_order[a] < sort_order[b]
end

-- Convert MAN_TEMPLATES into a more convenient data structure. We create a
-- table that for each man page stores a list of all associated man pages.
local man_friends = {}
for _, v in ipairs(MAN_TEMPLATES) do
	if type(v) == 'table' then
		local list = {v.parent}
		for _, v2 in ipairs(v) do
			man_friends[v2] = list
			list[#list + 1] = v2:match('[^/]*$')
		end
		if v.parent ~= nil then
			man_friends[v.parent:match('[^/]*$')] = list
		end
	end
end

function generate_man(output_path, template_path, pconf)
	local fname = template_path:match('[^/]*$')
	local fout = io.open(output_path..'/'..fname, 'w')
	local module, see_also = nil, {}
	local synopsis_block = nil
	print('GEN', output_path:gsub('^gen/../', '')..'/'..fname)

	-- Write license header
	fout:write(MAN_HEADER_TEXT)

	local linenum = 1
	for line in io.lines(template_path) do

		-- Handle .\"SS{ and .\"SS} suffixed synopses
		if synopsis_block == nil then
			if line:match('^%.\\"SS{$') then
				synopsis_block = ''
				goto continue
			end
		else
			if line:match('^%.\\"SS}$') then
				for pset in pconf:iter() do
					local tmp = synopsis_block
					for k, v in pairs(pset.params) do
						tmp = tmp:gsub('%$'..k..'%$', v)
					end
					fout:write(tmp)
				end
				synopsis_block = nil
			else
				synopsis_block = synopsis_block..line..'\n'
			end
			goto continue
		end

		-- If SEE ALSO section is included, don't generate it
		if line:match('^%.SH SEE ALSO$') then
			see_also = nil
		end

		-- Add any referenced man pages to SEE ALSO
		if see_also ~= nil then
			local name, page = line:match('^%.BR%s+([%w%d_]+)%s+%((%d+)%)')
			if (name and page) ~= nil then
				see_also[name..'('..page..')'] = true
			end
		end

		-- Expand snippets, if any
		if Snippet.expand_man(line, fout) then
			goto continue
		end

		-- Read module name from .\"M <module>
		if module == nil then
			module = line:match('^%s*%.\\"M (.*)%s*$')
			if module ~= nil then
				goto continue
			end
		end

		-- Expand .\"NAME suffix...
		if line:match('^%s*%.\\"NAME$') or line:match('^%s*%.\\"NAME%s') then
			local args = line:match('^%s*%.\\"NAME%s+(.*)$')
			local base = fname:gsub('%..*', '')
			fout:write('.SH NAME\n', base)
			for pset in pconf:iter() do
				fout:write(',\n', base, pset.params.SUFFIX)
			end
			if args ~= nil then
				for suffix in args:gmatch('[^%s]*') do
					fout:write(',\n', base, suffix)
				end
			end
			fout:write('\n')
			goto continue
		end

		fout:write(line, '\n')

		::continue::
		linenum = linenum + 1
	end

	-- Add SEE ALSO section at the end
	if see_also ~= nil then
		local dirname = template_path:match('.*/')
		fout:write('.SH SEE ALSO\n', '.ad l\n')
		if fname ~= 'libstaple.7' then
			see_also['libstaple(7)'] = true
		end
		if module ~= nil then
			if man_friends[fname] == nil then
				goto skip_friends
			end
			for _, f in ipairs(man_friends[fname]) do
				if fname == f then
					goto continue
				end
				local name, page = f:match('([%w%d_]+)%.(%d+)')
				if (name and page) ~= nil then
					see_also[name..'('..page..')'] = true
				end
				::continue::
			end
			::skip_friends::
			if fname ~= 'sp_'..module..'.7' then
				see_also['sp_'..module..'(7)'] = true
				see_also[fname] = nil
			end
		end

		-- Convert see_also hashset to list and sort it
		local see_also_list = {}
		for str, _ in pairs(see_also) do
			see_also_list[#see_also_list + 1] = str
		end
		table.sort(see_also_list, see_also_comp)

		-- Write see_also_list to the file
		for i, str in pairs(see_also_list) do
			local name, page = str:match('([%w%d_]+)%((%d+)%)')
			fout:write('.BR '..name..' ('..page..')',
			           i < #see_also_list and ',\n' or '\n')
		end
	end
	fout:close()
end
