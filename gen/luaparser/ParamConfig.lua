--------------------------------------------------------------------------------
-- An aggregation of paramsets defining the configuration for all source files,
-- header files and man pages. A single ParamConfig is used to generate
-- everything. Internally, it is a simple list of paramsets
--------------------------------------------------------------------------------

ParamConfig = {
	psets = {},
	iter_order = nil,
	iter_order_desync = true
}

-- Creates a new list of parameter sets
function ParamConfig:new(data)
	local o = {
		psets = {},
	}
	setmetatable(o, self)
	self.__index = self
	self.__len = function() return #o.psets end
	if data then
		for stdc, ptable in pairs(data) do
			o:read_ptable(ptable, stdc)
		end
	end
	return o
end

function ParamConfig:read_ptable(ptable, stdc)
	-- Parse parameter dicts into ParamSets
	for _, params in ipairs(ptable) do
		-- By convention, split FMT param into FMT_STR and FMT_ARGS.
		-- This allows the data table to be more clean and concise.
		if params.FMT then
			local fmt_count = 0
			for _ in params.FMT:gmatch('%%') do
				fmt_count = fmt_count + 1
			end
			params.FMT_STR  = params.FMT:gsub('%%', '%%%%')
			params.FMT_ARGS = 'elem'..(', elem'):rep(fmt_count - 1)
			params.FMT      = nil
		end

		-- By convention, the INCLUDE param is a list of header file
		-- names which should be included. This param can be defined
		-- specifically for each pset, or globally for all of them (or
		-- both, in which case specific lists override the global one).
		local include = ptable.INCLUDE
		if params.INCLUDE then
			include        = params.INCLUDE
			params.INCLUDE = nil
		end

		self.psets[#self.psets + 1] = ParamSet:new(params, stdc, include)
	end
end

-- Appends a ParamSet to the list
function ParamConfig:add(pset)
	self.psets[#self.psets + 1] = pset
	self.iter_order_desync = true
end

-- Sorts the psets in deterministic order and caches it in self.iter_order
function ParamConfig:update_iter_order()
	self.iter_order = {}
	for i, v in ipairs(self.psets) do
		self.iter_order[#self.iter_order + 1] = i
	end
	local function pset_str(pset)
		return pset.stdc..pset:get('SUFFIX')..pset:hash()
	end
	table.sort(self.iter_order,
		function(a, b)
			return pset_str(self.psets[a]) < pset_str(self.psets[b])
		end
		)
	self.iter_order_desync = false
end

-- Returns generic for loop iterator
function ParamConfig:iter()
	if self.iter_order_desync then
		self:update_iter_order()
	end
	local idx = 0
	return function()
		idx = idx + 1
		if idx <= #self.psets then
			return self.psets[self.iter_order[idx]]
		end
	end
end

function ParamConfig:print()
	print('ParamConfig '..tostring(self)..':')
	for i, v in ipairs(self.psets) do
		io.write('['..i..']:\t')
		v:print()
	end
end


