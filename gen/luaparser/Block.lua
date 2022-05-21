require(DIRNAME..'luaparser.ParamSet')
require(DIRNAME..'luaparser.ParamConfig')

Block = {
	header     = nil, -- file header (typically license notice)
	includes   = nil, -- list of include strings
	body       = {},  -- block contents (typically function definition)
	params     = nil  -- cache for Block:filter_paramsets()
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
		header   = header,
		includes = includes,
		body     = body,
		params   = {}
	}
	setmetatable(o, self)
	self.__index = self

	-- Cache all parameters that occur within the block
	for _, line in ipairs(o.body) do
		for param in line:gmatch('%$([%w_]*)%$') do
			o.params[param] = true
		end
	end
	
	return o
end

-- Given a list of paramsets, cross-references each set with Block.params and
-- returns a maximally compacted, sufficient "subset" of the input list. This
-- allows Block:write_expand() to avoid creating the same files multiple times.
function Block:filter_paramconf(pconf)

	-- Populate filtered_set with filtered ParamSet hashes
	local filtered = {}
	for pset in pconf:iter() do
		local filtered_pset = ParamSet:new(nil, pset.stdc, pset.includes)
		for k, v in pset:iter() do
			if self.params[k] then
				filtered_pset:set(k, v)
			end
		end
		if #filtered_pset ~= 0 then
			filtered[filtered_pset:hash()] = filtered_pset
		end
	end

	-- Convert filtered set to a filtered list of paramsets
	local filtered_pconf = ParamConfig:new()
	for _, filtered_pset in pairs(filtered) do
		filtered_pconf:add(filtered_pset)
	end

	return filtered_pconf
end

-- Given a list of paramsets, creates files by expanding block parameters in
-- accordance with each paramset.
function Block:write_expand(output_path, pconf)

	-- Filtering paramsets should guarantee that each paramset will
	-- correspond to exactly 1 output file
	local filtered_pconf = self:filter_paramconf(pconf)
	local fpath

	-- If no parameters are needed for this block, guarantee that it will
	-- still export once by adding an empty paramset
	if #filtered_pconf == 0 then
		filtered_pconf:add(ParamSet:new())
	end

	for pset in filtered_pconf:iter() do
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
					fpath = output_path..'/'..fname..'.c'
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
