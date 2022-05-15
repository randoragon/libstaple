require(DIRNAME..'luaparser.ParamSet')
require(DIRNAME..'luaparser.ParamSetList')

Block = {
	text   = nil, -- raw string of the entire block
	params = nil  -- cache for Block:filter_paramsets()
}

-- Constructs a new block object out of its raw textual form.
function Block:new(text)
	local o = {
		text = text,
		params = {}
	}
	setmetatable(o, self)
	self.__index = self

	-- Cache the names of parameters which occur within the block
	for name in o.text:gmatch('%$([%w_]+)%$') do
		o.params[name] = true
	end

	-- MODULE by convention is always required
	o.params.MODULE = true

	return o
end

-- Given a list of paramsets, cross-references each set with Block.params and
-- returns a maximally compacted, sufficient "subset" of the input list. This
-- allows Block:write_expand() to avoid creating the same files multiple times.
-- As a second value, the value of the MODULE parameter is returned, because
-- it plays a critical role in establishing the output file path.
function Block:filter_paramsets(psets)

	-- Populate filtered_set with filtered ParamSet hashes
	local filtered = {}
	for _, pset in ipairs(psets.list) do
		local filtered_pset = ParamSet:new()
		for k, v in pset:iter() do
			if self.params[k] then
				filtered_pset:set(k, v)
			end
		end
		filtered[filtered_pset:hash()] = true
	end

	-- Convert filtered set to a filtered list of paramsets
	local filtered_psets = ParamSetList:new()
	for k, _ in pairs(filtered) do
		filtered_psets:add(ParamSet.unhash(k))
	end

	return filtered_psets
end

-- Given a list of paramsets, creates files by expanding block parameters in
-- accordance with each paramset.
function Block:write_expand(psets)

	-- Filtering paramsets guarantees that each paramset will correspond to
	-- exactly 1 output file.
	local filtered_psets = self:filter_paramsets(psets)
	local fpath

	for pset in filtered_psets:iter() do
		local text = self.text

		-- Expand parameters ($PARAM$)
		for k, v in pset:iter() do
			text = text:gsub('%$'..k..'%$', v)
		end
		assert(not text:match('%$[%w_]+%$'), 'unmatched param in text:\n'..text)

		-- Extract file path from function name
		local fname = text:match('[^%w_]+([%w_]+)%(')
		if fname then
			fpath = SRCDIR..pset:get('MODULE')..'/'..fname..'.c'
		end
		assert(fpath, 'undetected function name in text:\n'..text)

		-- Save the file
		print('GEN', fpath)
		local fout = io.open(fpath, 'w')
		fout:write(text)
		fout:close()
	end
end

-- For debugging
function Block:print()
	print('Block '..tostring(self)..':')

	print('.params: {')
	for k, _ in pairs(self.params) do
		print('\t'..k)
	end
	print('}')

	print('.text:')
	print('\t'..self.text:gsub('\n', '\n\t'))
end
