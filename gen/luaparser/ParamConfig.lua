require(DIRNAME..'luaparser.ParamSet')

ParamConfig = {
	psets = {}
}

-- Creates a new list of parameter sets
function ParamConfig:new(data)
	local o = {
		psets = {}
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
end

-- Returns generic for loop iterator
function ParamConfig:iter()
	local idx = 0
	return function()
		idx = idx + 1
		if idx <= #self.psets then
			return self.psets[idx]
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
