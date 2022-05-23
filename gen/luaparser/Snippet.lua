--------------------------------------------------------------------------------
-- A simple static class exposing a function for expanding snippets.
--------------------------------------------------------------------------------

Snippet = {}

-- Expands snippet lines (/*. FILENAME [ARG1...ARGN] */)
--
-- Intakes a line and a list. Expands the snippet (if one is detected) and
-- appends the proper content to the list on a line-by-line basis.
-- Returns true if a snippet was found, false otherwise.
function Snippet.expand(line, list)
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
