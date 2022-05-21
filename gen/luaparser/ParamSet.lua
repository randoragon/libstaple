ParamSet = {
	params   = {},
	stdc     = 'C89',
	includes = {}
}

-- Create a new parameter set.
function ParamSet:new(params, stdc, includes)
	local o = {
		params   = params   or {},
		stdc     = stdc     or 'C89',
		includes = includes or {}
	}
	setmetatable(o, self)
	self.__index = self
	self.__len = function()
		local count = 0
		for _ in pairs(o.params) do
			count = count + 1
		end
		return count
	end
	return o
end

-- Sets the value of a parameter. If the parameter does not exist, it is added
-- to the set.
function ParamSet:set(key, value)
	self.params[key] = value
end

-- Returns the value of a parameter.
function ParamSet:get(key)
	return self.params[key]
end

-- Returns generic for loop iterator
function ParamSet:iter()
	return pairs(self.params)
end

-- Returns a string hash of the parameter dictionary.
function ParamSet:hash()
	local strings = {}

	local function kv2str(key, value)
		assert(type(key)   == 'string', 'key must be a string, not: '..type(key))
		assert(type(value) == 'string', 'value must be a string, not: '..type(value))
		assert(not key:match('[^%w_]'), 'invalid param name: '..key)
		assert(not key:match('[@]'),    'key/separator collision: '..key)
		assert(not value:match('%$'),   'invalid param value: '..value)
		assert(not value:match('[@]'),  'value/separator collision: '..value)
		return key..'$'..value
	end

	for k, v in pairs(self.params) do
		strings[#strings + 1] = kv2str(k, v)
	end
	table.sort(strings)
	return table.concat(strings, '@')
end

-- For debugging
function ParamSet:print()
	print('ParamSet '..tostring(self))
	print('\tstdc:     '..self.stdc)

	io.write('\tincludes: { ')
	for _, v in ipairs(self.includes) do
		io.write(v..' ')
	end
	print(' }')

	io.write('\tparams:   { ')
	for k, v in pairs(self.params) do
		io.write(k..'='..v..' ')
	end
	print(' }')
end
