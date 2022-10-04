--------------------------------------------------------------------------------
-- This file exposes the generate_h function used for generating C header files.
--------------------------------------------------------------------------------

require(DIRNAME..'luaparser.ParamConfig')
require(DIRNAME..'luaparser.STDCGuard')

function generate_h(output_path, template_path, pconf)
	print('GEN', template_path)
	local fname = template_path:match('[^/]*$')
	local fout = io.open(output_path..'/'..fname, 'w')
	local inside_desc = false
	local header_guard

	-- Write license header
	fout:write(C_HEADER_TEXT)

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
			local last_pset
			for pset in pconf:iter() do
				-- Write one line for each distinct paramset
				local expanded = line
				for k, v in pset:iter() do
					expanded = expanded:gsub('%$'..k..'%$', v)
				end
				assert(not expanded:match('%$[%w_]+%$'), 'unmatched param in line '..linenum..': '..line)

				-- Avoid repeating the same lines
				if not already_written[expanded] then
					if not last_pset or pset.stdc ~= last_pset.stdc then
						if last_pset then
							fout:write(STDCGuard.close(last_pset.stdc, false))
						end
						fout:write(STDCGuard.open(pset.stdc))
						fout:write(expanded, '\n')
					else
						fout:write(expanded, '\n')
					end
					already_written[expanded] = true
					last_pset = pset
				end
			end
			if last_pset then
				fout:write(STDCGuard.close(last_pset.stdc))
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
