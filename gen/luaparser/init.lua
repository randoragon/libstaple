require(DIRNAME..'luaparser.Block')
require(DIRNAME..'luaparser.ParamSet')
require(DIRNAME..'luaparser.ParamConfig')

-- Expands snippet lines (/*. FILENAME [ARG1...ARGN] */)
--
-- Intakes a line and a list. Expands the snippet (if one is detected) and
-- appends the proper content to the list on a line-by-line basis.
-- Returns true if a snippet was found, false otherwise.
local function expand_snippet(line, list)
	local indent, snip, argstr = line:match('^(%s*)/%*%.%s*([%w_]+)%s*(.*)%s*%*/')
	if snip then
		local snip_linenum = 1
		for snip_line in io.lines(SNIPPETDIR..snip) do
			local args, i = {}, 1
			for word in argstr:gmatch('([^%s]*)') do
				snip_line = snip_line:gsub('%$'..i..'%$', word)
				i = i + 1
			end
			assert(not snip_line:match('%$%d+%$'), 'unmatched args left in snippet '..snip..' on line '..snip_linenum..':\n\t'..snip_line)
			list[#list + 1] = indent..snip_line
			snip_linenum = snip_linenum + 1
		end
		return true
	end
	return false
end

-- Intakes a C version string (C89, C99, ...), and returns the opening of an
-- adequate preprocessor guard.
function stdc_guard_open(stdc)
	local STDC_VALUES = {
		C95 = '199409L',
		C99 = '199901L',
		C11 = '201112L',
		C17 = '201710L'
	}
	if stdc ~= 'C89' then
		local val = STDC_VALUES[stdc]
		return '#if defined(__STDC_VERSION__) && '..
		'(__STDC_VERSION__ >= '..val..')\n'
	end
	return ''
end

-- Intakes a C version string (C89, C99, ...) and returns the closing of an
-- adequate preprocessor guard.
function stdc_guard_close(stdc, prevent_empty)
	if stdc ~= 'C89' then
		if prevent_empty then
			-- ISO C forbids empty translation units, so add a dummy
			-- meaningless line to prevent compilation errors
			return '\n\n#else\n'..
			       'typedef int prevent_empty_translation_unit;\n'..
			       '#endif'
		else
			return '#endif\n'
		end
	end
	return ''
end

-- Detect template type and generate appropriate files.
--
-- pconf is a ParamConfig object containing a list of ParamSets. Each ParamSet
-- should contain key-value pairs corresponding to the parameters inside the
-- input template file.
function generate(output_path, template_path, pconf)
	local func
	if template_path:match('.*%.c$') then
		func = generate_c
	elseif template_path:match('.*%.h$') then
		func = generate_h
	elseif template_path:match('.*%.%d$') then
		func = generate_man
	else
		io.stderr:write('unknown template type: "', template_path:gsub('.*/', ''), '"\n')
		return
	end
	func(output_path, template_path, pconf)
end

-- Generate subroutine for C source files.
function generate_c(output_path, template_path, pconf)
	local base_includes = {}
	local inside_block = false
	local body, includes

	local linenum = 1
	for line in io.lines(template_path) do
		if not inside_block then

			-- Collect base includes
			if line:match('^#include') then
				base_includes[#base_includes + 1] = line
				goto continue
			end

			-- Detect beginnings of blocks
			if line:match('^/%*F{.*%*/') then
				inside_block = true
				body = {}
				includes = {table.unpack(base_includes)}
				goto continue
			end

		else -- if inside a block:

			-- Collect block-specific includes
			if line:match('^#include') then
				includes[#includes + 1] = line
				goto continue
			end

			-- Expand snippets, if any
			if expand_snippet(line, body) then
				goto continue
			end

			-- End of block reached
			if line:match('^/%*F}%s*%*/') then
				local block = Block:new(HEADER_TEXT, includes, body)
				block:write_expand(output_path, pconf)
				inside_block = false
				goto continue
			end

			body[#body + 1] = line
		end

		::continue::
		linenum = linenum + 1
	end
end

-- Generate subroutine for C header files.
function generate_h(output_path, template_path, pconf)
	print('GEN', template_path)
	local fname = template_path:match('[^/]*$')
	local fout = io.open(output_path..'/'..fname, 'w')
	local inside_desc = false
	local header_guard

	-- Write license header
	fout:write(HEADER_TEXT)

	local linenum = 1
	for line in io.lines(template_path) do

		-- Parse description blocks
		if not inside_desc and not header_guard then
			-- Detect start of a description block
			header_guard = line:match('^/%*H{%s*([%w_]*)%s*%*/')
			if header_guard then
				inside_desc = true
				fout:write('#ifndef ', header_guard, '\n',
				           '#define ', header_guard, '\n\n')
				goto continue
			end
		else -- if inside a description block
			if line:match('^/%*H}.*%*/') then -- End of description reached
				inside_desc = false
				goto continue
			end
		end

		-- Expand parameters, if any
		if line:match('%$[%w_]*%$') then
			local already_written = {}
			for pset in pconf:iter() do
				-- Write one line for each distinct paramset
				local expanded = line
				for k, v in pset:iter() do
					expanded = expanded:gsub('%$'..k..'%$', v)
				end
				assert(not expanded:match('%$[%w_]+%$'), 'unmatched param in line '..linenum..': '..line)

				-- Avoid repeating the same lines
				if not already_written[expanded] then
					fout:write(stdc_guard_open(pset.stdc))
					fout:write(expanded, '\n')
					fout:write(stdc_guard_close(pset.stdc, false))
					already_written[expanded] = true
				end
			end
		else
			fout:write(line, '\n')
		end

		::continue::
		linenum = linenum + 1
	end

	-- Close the header guard
	assert(header_guard, 'description block not found: '..template_path)
	fout:write('\n#endif /* ', header_guard, ' */')
	fout:close()
end
