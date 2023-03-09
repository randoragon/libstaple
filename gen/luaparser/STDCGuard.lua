--------------------------------------------------------------------------------
-- A simple static class exposing functions for generating __STDC_VERSION__
-- preprocessor guards.
--------------------------------------------------------------------------------

STDCGuard = {}

-- Intakes a C version string (C89, C99, ...), and returns the opening of an
-- adequate preprocessor guard.
function STDCGuard.open(stdc)
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
function STDCGuard.close(stdc, prevent_empty)
	if stdc ~= 'C89' then
		if prevent_empty then
			-- ISO C forbids empty translation units, so add a dummy
			-- meaningless line to prevent compilation errors
			return '\n#else\n'..
			       'typedef int prevent_empty_translation_unit;\n'..
			       '#endif\n'
		else
			return '#endif\n'
		end
	end
	return ''
end
