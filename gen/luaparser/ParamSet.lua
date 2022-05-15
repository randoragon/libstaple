ParamSet = {
	dict = {}
}

-- Create a new parameter set.
function ParamSet:new(params)
	local o = { dict = params or {} }
	setmetatable(o, self)
	self.__index = self
	return o
end

-- Sets the value of a parameter. If the parameter does not exist, it is added
-- to the set.
function ParamSet:set(key, value)
	self.dict[key] = value
end

-- Returns the value of a parameter.
function ParamSet:get(key)
	return self.dict[key]
end

-- Returns generic for loop iterator
function ParamSet:iter()
	return pairs(self.dict)
end

-- Returns a string hash of the parameter dictionary.
function ParamSet:hash()
	local strings = {}

	local function kv2str(key, value)
		assert(type(key)   == 'string', 'key must be a string')
		assert(type(value) == 'string', 'value must be a string')
		assert(not key:match('[^%w_]'), 'invalid param name: '..key)
		assert(not key:match('[@]'),    'key/separator collision: '..key)
		assert(not value:match('%$'),   'invalid param value: '..value)
		assert(not value:match('[@]'),  'value/separator collision: '..value)
		return key..'$'..value
	end

	for k, v in pairs(self.dict) do
		strings[#strings + 1] = kv2str(k, v)
	end
	table.sort(strings)
	return table.concat(strings, '@')
end

-- Inverse of ParamSet:hash()
function ParamSet.unhash(hash)
	local dict = {}

	local function str2kv(str)
		key, value = str:match('(.*)%$(.*)')
		assert(key,   'str2kv failed on "'..str..'" (key)')
		assert(value, 'kvr2kv failed on "'..str..'" (value)')
		return key, value
	end

	for str in hash:gmatch('[^@]*') do
		local k, v = str2kv(str)
		dict[k] = v
	end
	return ParamSet:new(dict)
end

-- For debugging
function ParamSet:print()
	io.write('ParamSet '..tostring(self)..' { ')
	for k, v in pairs(self.dict) do
		io.write(k..'="'..v..'" ')
	end
	print('}')
end
