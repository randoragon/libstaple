--------------------------------------------------------------------------------
-- The luaparser module handles generating C source files, C header files and
-- man pages for the entire project.
--------------------------------------------------------------------------------

require(DIRNAME..'luaparser.generate_c')
require(DIRNAME..'luaparser.generate_h')
require(DIRNAME..'luaparser.generate_man')

-- Detect template type and generate appropriate files.
--
-- pconf is a ParamConfig object containing a list of ParamSets. Each ParamSet
-- should contain key-value pairs corresponding to the parameters inside the
-- input template file.
function generate(output_path, template_path, pconf)
	local func
	if template_path:match('.*%.c$') then
		func = generate_c
	elseif template_path:match('.*%.h$') then
		func = generate_h
	elseif template_path:match('.*%.%d$') then
		func = generate_man
	else
		error('unknown template type: "', template_path:gsub('.*/', ''), '"\n')
	end
	func(output_path, template_path, pconf)
end

