require(DIRNAME..'luaparser.ParamSet')

ParamSetList = {
	list = {}
}

-- Creates a new list of parameter sets
function ParamSetList:new(list)
	local o = { list = {} }
	setmetatable(o, self)
	self.__index = self
	if list then
		for _, dict in ipairs(list) do
			o.list[#o.list + 1] = ParamSet:new(dict)
		end
	end
	return o
end

-- Appends a ParamSet to the list
function ParamSetList:add(pset)
	self.list[#self.list + 1] = pset
end

-- Returns generic for loop iterator
function ParamSetList:iter()
	local idx = 0
	return function()
		idx = idx + 1
		if idx <= #self.list then
			return self.list[idx]
		end
	end
end

function ParamSetList:print()
	print('ParamSetList '..tostring(self)..':')
	for i, v in ipairs(self.list) do
		io.write('['..i..']:\t')
		v:print()
	end
end
