#!/usr/bin/env lua

-- This program generates the C source files from a rudimentary template format.
-- This was necessary, because the C preprocessor does not offer support for
-- advanced file splitting.

-- GLOBAL CONFIGURATION
SRCDIR      = '../src/'
SNIPPETDIR  = 'snippets/'
TEMPLATEDIR = 'templates/'
HEADER_TEXT = io.open(SNIPPETDIR..'C_LICENSE_HEADER', 'r'):read('a')

-- List of file paths to templates, relative to TEMPLATEDIR
INPUT_FILES = arg[1] and arg or { 'stack', 'queue' }

-- Suffixed forms to generate
PARAMS = {
	{ TYPE='char'          , SUFFIX='c' , FMT_STR="%%hd\\t'%%c'", FMT_ARGS='elem, elem' },
	{ TYPE='short'         , SUFFIX='s' , FMT_STR='%%hd'        , FMT_ARGS='elem'       },
	{ TYPE='int'           , SUFFIX='i' , FMT_STR='%%d'         , FMT_ARGS='elem'       },
	{ TYPE='long'          , SUFFIX='l' , FMT_STR='%%ld'        , FMT_ARGS='elem'       },
	{ TYPE='unsigned char' , SUFFIX='uc', FMT_STR="%%hd\\t'%%c'", FMT_ARGS='elem, elem' },
	{ TYPE='signed char'   , SUFFIX='sc', FMT_STR="%%hd\\t'%%c'", FMT_ARGS='elem, elem' },
	{ TYPE='unsigned short', SUFFIX='us', FMT_STR='%%hu'        , FMT_ARGS='elem'       },
	{ TYPE='unsigned int'  , SUFFIX='ui', FMT_STR='%%u'         , FMT_ARGS='elem'       },
	{ TYPE='unsigned long' , SUFFIX='ul', FMT_STR='%%lu'        , FMT_ARGS='elem'       },
	{ TYPE='float'         , SUFFIX='f' , FMT_STR='%%g'         , FMT_ARGS='elem'       },
	{ TYPE='double'        , SUFFIX='d' , FMT_STR='%%g'         , FMT_ARGS='elem'       },
	{ TYPE='long double'   , SUFFIX='ld', FMT_STR='%%Lg'        , FMT_ARGS='elem'       },
	-- str and strn suffixed functions are implemented manually
}

local function tcopy(table)
	local ret
	if table ~= nil then
		ret = {}
		for k, v in pairs(table) do
			ret[k] = v
		end
	end
	return ret
end

local function tequal(t1, t2)
	for k, v in pairs(t1) do
		if t2[k] ~= v then return false end
	end
	for k, v in pairs(t2) do
		if t1[k] ~= v then return false end
	end
	return true
end

-- Generates C source file(s) from the template format.
--
-- params must contain key-value pairs that match the format strings inside
-- the input template file.
local function generate(fpath_in, params)

	local include_lines = {}
	local default_includes = {}
	local includes = {}
	local inside = false
	local fpath_out, buf

	local linenum = 1
	for line in io.lines(fpath_in) do

		if not inside then
			-- Parse C include statements
			if line:match('^#include%s') then
				table.insert(include_lines, line)

			-- Detect beginnings of file blocks
			elseif line:match('^/%*F{.*%*/') then
				inside = true

				-- Write the header and explicitly stated includes
				buf = HEADER_TEXT..'\n'
				for k, v in ipairs(default_includes) do
					if v then
						buf = buf..include_lines[k]..'\n'
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

				-- Set to nil to indicate that filename hasn't been parsed yet
				fpath_out = nil
			else
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
				end
			end
		else
			-- Expand snippet lines: /*. FILENAME [ARG1...ARGN] */
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

			-- End of file block detected
			elseif line:match('^/%*F}%s*%*/') then
				-- Find parameters relevant to the buffer, then filter out the
				-- irrelevant ones to avoid regenerating the same source file
				-- several times
				local param_in_buf = {} -- just for caching
				local filtered_params = {}
				for _, i in ipairs(params) do
					filtered = {}
					for k, v in pairs(i) do
						if param_in_buf[k] == nil then
							param_in_buf[k] = not not buf:match('%$'..k..'%$')
						end
						if param_in_buf[k] then
							filtered[k] = v
						end
					end

					-- If the filtered row is not yet on the list, add it
					local already_in = false
					for _, j in ipairs(filtered_params) do
						if tequal(filtered, j) then
							already_in = true
							break
						end
					end
					if not already_in then
						filtered_params[#filtered_params + 1] = filtered
					end
				end

				-- Generate a new file for each set of params
				for i = 1, #filtered_params do
					local p = filtered_params[i]
					local text = buf


					-- Expand inline parameters: $PARAM$
					for k, v in pairs(p) do
						text = text:gsub('%$'..k..'%$', v)
					end
					assert(not text:match('%$[%w_]+%$'), 'unmatched param in text:\n'..text)

					-- Extract file path from function name
					local fname = text:match('[^%w_]+([%w_]+)%(')
					if fname then
						fpath_out = SRCDIR..params[i].MODULE..'/'..fname..'.c'
					end
					assert(fpath_out, 'undetected function name in text:\n'..text)

					-- Write to file
					local fout = io.open(fpath_out, 'w')
					fout:write(text)
					fout:close()
					print('GEN', fpath_out)
				end
				inside = false
			else
				-- Normal line of code; rewrite as-is
				buf = buf..line..'\n'
			end
		end
		linenum = linenum + 1
	end
end

-- Generate source files
for i, file in ipairs(INPUT_FILES) do
	-- Add MODULE parameter to PARAMS
	for _, i in ipairs(PARAMS) do
		i.MODULE = file
	end
	generate(TEMPLATEDIR..file, PARAMS)
end
