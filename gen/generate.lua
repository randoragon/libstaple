#!/usr/bin/env lua

-- This program generates the C source files from a rudimentary template format.
-- This was necessary, because the C preprocessor does not offer support for
-- advanced file splitting.

-- GLOBAL CONFIGURATION
SRCDIR      = '../src/'
SNIPPETDIR  = 'snippets/'
TEMPLATEDIR = 'templates/'
INPUT_FILES = arg
HEADER_TEXT = io.open(SNIPPETDIR..'C_LICENSE_HEADER', 'r'):read('a')

SUFFIXES = {
	c    = 'char',
	s    = 'short',
	i    = 'int',
	l    = 'long',
	uc   = 'unsigned char',
	sc   = 'signed char',
	us   = 'unsigned short',
	ui   = 'unsigned int',
	ul   = 'unsigned long',
	f    = 'float',
	d    = 'double',
	ld   = 'long double',
	str  = 'char *',
	strn = 'char *'
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

-- Expands parameter tables into lists of combinations.
-- For example, the call:
--     combine_params { A={0,1}, B='ham', C={'oh', 'hi', 'mark'} }
-- would produce something like:
--     {
--       {A=0, B='ham', C='oh'},
--       {A=0, B='ham', C='hi'},
--       {A=0, B='ham', C='mark'},
--       {A=1, B='ham', C='oh'},
--       {A=1, B='ham', C='hi'},
--       {A=1, B='ham', C='mark'}
--     }
function combine_params(params, prefix, lastkey)
	local key, val = next(params, lastkey)
	if key == nil then
		return {prefix}
	end

	-- expand current key
	local ret = {}
	if type(val) ~= 'table' then
		ret[1] = tcopy(prefix) or {}
		ret[1][key] = val
		ret = combine_params(params, ret[1], key)
	else
		for _, elem in ipairs(val) do
			ret[#ret + 1] = tcopy(prefix) or {}
			ret[#ret][key] = elem
			local new = combine_params(params, ret[#ret], key)
			table.move(new, 1, #new, #ret, ret)
		end
	end

	return ret
end

-- Generates C source file(s) from the template format.
--
-- params must contain key-value pairs that match the format strings inside
-- the input template file.
local function generate(fpath_in, params)

	-- params by convention must contain at least this, because it is used for
	-- constructing output file paths
	assert(params['MODULE'], 'key "MODULE" not found in params')
	assert(params['SUFFIX'], 'key "SUFFIX" not found in params')

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
				-- Generate a new file for each combination of params
				for _, p in ipairs(combine_params(params)) do
					local text = buf

					-- Expand inline parameters: $PARAM$
					for k, v in pairs(p) do
						text = text:gsub('%$'..k..'%$', v)
					end
					assert(not text:match('%$[%w_]+%$'), 'unmatched param in text:\n'..text)

					-- Extract file path from function name
					if fpath_out == nil then
						local fname = text:match('[^%w_]+([%w_]+)%(')
						if fname then
							fpath_out = SRCDIR..params['MODULE']..'/'..fname..'.c'
						end
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

for i,file in ipairs(INPUT_FILES) do
	for s, t in pairs(SUFFIXES) do
		params = {
			MODULE = file,
			SUFFIX = s,
			TYPE   = t
		}
		generate(TEMPLATEDIR..file, params)
	end
end
