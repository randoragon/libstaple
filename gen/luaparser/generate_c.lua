--------------------------------------------------------------------------------
-- This file exposes the generate_c function used for generating C source files.
--------------------------------------------------------------------------------

require(DIRNAME..'luaparser.FDef')
require(DIRNAME..'luaparser.Snippet')

function generate_c(output_path, template_path, pconf)
	local base_includes = {}
	local inside_fdef = false
	local body, includes

	local linenum = 1
	for line in io.lines(template_path) do
		if not inside_fdef then

			-- Collect base includes
			if line:match('^#include') then
				base_includes[#base_includes + 1] = line
				goto continue
			end

			-- Detect beginnings of function definitions
			if line:match('^/%*F{.*%*/') then
				inside_fdef = true
				body = {}
				includes = {table.unpack(base_includes)}
				goto continue
			end

		else -- if inside a function definition:

			-- Collect function-specific includes
			if line:match('^#include') then
				includes[#includes + 1] = line
				goto continue
			end

			-- Expand snippets, if any
			if Snippet.expand_c(line, body) then
				goto continue
			end

			-- End of function definition reached
			if line:match('^/%*F}%s*%*/') then
				local fdef = FDef:new(C_HEADER_TEXT, includes, body)
				fdef:write_expand(output_path, pconf)
				inside_fdef = false
				goto continue
			end

			body[#body + 1] = line
		end

		::continue::
		linenum = linenum + 1
	end
end
