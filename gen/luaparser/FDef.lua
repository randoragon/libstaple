--------------------------------------------------------------------------------
-- This class is only used by generate_c.lua. It takes a function definition in
-- the template format, parses it, creates valid C source files and distributes
-- them to the right paths.
--------------------------------------------------------------------------------

require(DIRNAME..'luaparser.ParamConfig')
require(DIRNAME..'luaparser.ParamSet')
require(DIRNAME..'luaparser.STDCGuard')

FDef = {
	header     = nil, -- file header (typically license notice)
	includes   = nil, -- list of include strings
	body       = {},  -- function definition (list of lines)
	params     = nil  -- cache for FDef:filter_paramsets()
}

-- FDef constructor.
-- header: string containing the header (typically license)
-- includes: a list of function-specific include strings
-- body: the function definition as a list of lines (strings)
function FDef:new(header, includes, body)
	local o = {
		header   = header,
		includes = includes,
		body     = body,
		params   = {}
	}
	setmetatable(o, self)
	self.__index = self

	-- Cache all parameters that occur within the definition
	for _, line in ipairs(o.body) do
		for param in line:gmatch('%$([%w_]*)%$') do
			o.params[param] = true
		end
	end
	
	return o
end

-- Given a ParamConfig object, cross-references each pset with FDef.params and
-- returns a maximally compacted, sufficient "subset" of the config. This allows
-- FDef:write_expand() to avoid creating the same files multiple times.
function FDef:filter_paramconf(pconf)

	-- Populate filtered_set with filtered ParamSet hashes
	local filtered, sort_order = {}, {}
	for pset in pconf:iter() do
		local filtered_pset = ParamSet:new(nil, pset.stdc, pset.includes)
		for k, v in pset:iter() do
			if self.params[k] then
				filtered_pset:set(k, v)
			end
		end
		if #filtered_pset ~= 0 then
			local hash = filtered_pset:hash()
			filtered[hash] = filtered_pset
			sort_order[#sort_order + 1] = hash
		end
	end

	-- Convert filtered set to a filtered list of paramsets
	local filtered_pconf = ParamConfig:new()
	for _, hash in ipairs(sort_order) do
		filtered_pconf:add(filtered[hash])
	end

	return filtered_pconf
end

-- Given a ParamConfig, creates files by expanding parameters in
-- accordance with each ParamSet within.
function FDef:write_expand(output_path, pconf)

	-- Filtering paramsets should guarantee that each paramset will
	-- correspond to exactly 1 output file
	local filtered_pconf = self:filter_paramconf(pconf)

	-- If no parameters are needed for this fdef, guarantee that it will
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

		-- Write header
		fout:write(self.header)

		-- Add a C version guard, if necessary
		fout:write(STDCGuard.open(pset.stdc))

		-- Write include lines
		if self.includes then
			for _, include in ipairs(self.includes) do
				fout:write(include, '\n')
			end
		end
		if #pset.includes ~= 0 then
			for _, include in ipairs(pset.includes) do
				fout:write('#include <', include, '>\n')
			end
		end
		fout:write('\n')

		-- Write body
		fout:write(table.concat(body, '\n'))

		-- Close the C version guard, if necessary
		fout:write(STDCGuard.close(pset.stdc, true))

		fout:close()
	end
end

-- For debugging
function FDef:print()
	print('FDef '..tostring(self)..':')

	print('.header:')
	print('\t'..self.header:gsub('\n', '\n\t'))

	print('.includes:')
	for _, line in ipairs(self.includes) do
		print('\t'..line)
	end

	print('.body:')
	print('\t'..table.concat(self.body, '\n\t'))
end
