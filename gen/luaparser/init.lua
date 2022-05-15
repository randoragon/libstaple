require(DIRNAME..'luaparser.Block')
require(DIRNAME..'luaparser.ParamSet')
require(DIRNAME..'luaparser.ParamSetList')

-- Generates C source file(s) from the template format.
--
-- params must contain key-value pairs that match the format strings inside_block
-- the input template file.
function generate(template_path, psets)

	local include_lines    = {}
	local default_includes = {}
	local inside_block     = false -- self-explanatory
	local buf

	local linenum = 1
	for line in io.lines(template_path) do
		if not inside_block then

			-- Parse C include statements
			if line:match('^#include') then
				table.insert(include_lines, line)
				goto continue
			end

			-- Parse default includes statements: /*I idx1,idx2,... */
			local indices = line:match('/%*I%s+([%d,]+)%s+.*%*/')
			if indices then
				default_includes = {}
				for i = 1,#include_lines do
					default_includes[i] = false
				end
				for idxstr in indices:gmatch('([^,]*)') do
					idx = tonumber(idxstr)
					assert(include_lines[idx], 'index '..idx..' out of range on line '..linenum..':\n\t'..line)
					default_includes[idx] = true
				end
				goto continue
			end

			-- Detect beginnings of blocks
			if line:match('^/%*F{.*%*/') then
				inside_block = true

				-- Write the header and default includes
				buf = HEADER_TEXT..'\n'
				for i, v in ipairs(default_includes) do
					if v then
						buf = buf..include_lines[i]..'\n'
					end
				end

				-- Parse and write additional include indices, if any
				local indices = line:match('^/%*F{%s+([%d,]+).*%*/')
				if indices then
					for idxstr in indices:gmatch('([^,]*)') do
						idx = tonumber(idxstr)
						assert(include_lines[idx], 'index '..idx..' out of range on line '..linenum..':\n\t'..line)
						if not default_includes[idx] then
							buf = buf..include_lines[idx]..'\n'
						end
					end
				end
				buf = buf..'\n'
				goto continue
			end

		else -- if inside a block:

			-- Expand snippet lines (/*. FILENAME [ARG1...ARGN] */)
			local indent, snip, argstr = line:match('^(%s*)/%*%.%s*([%w_]+)%s*(.*)%s*%*/')
			if snip then
				-- Expand inner-snippet arguments
				local snip_linenum = 1
				for snip_line in io.lines(SNIPPETDIR..snip) do
					local args, i = {}, 1
					for word in argstr:gmatch('([^%s]*)') do
						snip_line = snip_line:gsub('%$'..i..'%$', word)
						i = i + 1
					end
					assert(not snip_line:match('%$%d+%$'), 'unmatched args left in snippet '..snip..' on line '..snip_linenum..':\n\t'..snip_line)
					buf = buf..indent..snip_line..'\n'
					snip_linenum = snip_linenum + 1
				end
				goto continue
			end

			-- End of block reached
			if line:match('^/%*F}%s*%*/') then
				local block = Block:new(buf)
				block:write_expand(psets)
				inside_block = false
				goto continue
			end

			buf = buf..line..'\n'
		end

		::continue::
		linenum = linenum + 1
	end
end
