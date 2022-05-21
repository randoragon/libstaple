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

-- Generates C source file(s) from the template format.
--
-- pconf is a ParamConfig object containing a list of ParamSets. Each ParamSet
-- should contain key-value pairs corresponding to the parameters inside the
-- input template file.
function generate(output_path, template_path, pconf)

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
		last_line = line
	end
end
