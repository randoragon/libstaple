require(DIRNAME..'luaparser.ParamSet')
require(DIRNAME..'luaparser.ParamConfig')

Block = {
	header     = nil, -- file header (typically license notice)
	includes   = nil, -- list of include strings
	body       = {},  -- block contents (typically function definition)
}

local STDC_VALUES = {
	C95 = '199409L',
	C99 = '199901L',
	C11 = '201112L',
	C17 = '201710L'
}

-- Constructs a new block object out of its raw textual form.
function Block:new(header, includes, body)
	local o = {
		header     = header,
		includes   = includes,
		body       = body,
	}
	setmetatable(o, self)
	self.__index = self
	return o
end

-- Given a list of paramsets, creates files by expanding block parameters in
-- accordance with each paramset.
function Block:write_expand(pconf)
	local already_expanded = {}

	for pset in pconf:iter() do
		local body = {table.unpack(self.body)}
		local fpath = nil

		for i = 1, #body do
			-- Expand parameters ($PARAM$)
			for k, v in pset:iter() do
				body[i] = body[i]:gsub('%$'..k..'%$', v)
			end
			assert(not body[i]:match('%$[%w_]+%$'), 'unmatched param in line '..i..': '..body[i])

			-- Extract file path from function name
			if not fpath then
				local fname = body[i]:match('[^%w_]+([%w_]+)%(')
				if fname then
					fpath = SRCDIR..pset:get('MODULE')..'/'..fname..'.c'
					if already_expanded[fpath] then
						goto skip_file
					end
				end
			end
		end
		assert(fpath, 'undetected function name in body:\n\t'..table.concat(body, '\n\t'))

		-- Open the output file
		print('GEN', fpath)
		local fout = io.open(fpath, 'w')

		-- Write block header
		fout:write(self.header)

		-- If the standard isn't C89, surround with #if directive
		if pset.stdc ~= 'C89' then

			local val = STDC_VALUES[pset.stdc]
			fout:write('#if defined(__STDC_VERSION__) && '..
			           '(__STDC_VERSION__ >= '..val..')\n')
		end

		-- Write include lines
		if self.includes then
			for _, include in ipairs(self.includes) do
				fout:write(include..'\n')
			end
		end
		if pset:get('INCLUDE') then
			for include in pset:get('INCLUDE'):gmatch('[^,]*') do
				fout:write('#include <'..include..'>\n')
			end
		end
		fout:write('\n')

		-- Write body
		fout:write(table.concat(body, '\n'))

		-- If the standard isn't C89, terminate the #if directive
		if pset.stdc ~= 'C89' then
			-- ISO C forbids empty translation units, so add a dummy
			-- meaningless line to prevent compilation errors
			fout:write('\n\n#else\n')
			fout:write('typedef int prevent_empty_translation_unit;\n')
			fout:write('#endif')
		end

		fout:close()
		already_expanded[fpath] = true
		::skip_file::
	end
end

-- For debugging
function Block:print()
	print('Block '..tostring(self)..':')

	print('.header:')
	print('\t'..self.header:gsub('\n', '\n\t'))

	print('.includes:')
	for _, line in ipairs(self.includes) do
		print('\t'..line)
	end

	print('.body:')
	print('\t'..table.concat(self.body, '\n\t'))
end
