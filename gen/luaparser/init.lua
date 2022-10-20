--------------------------------------------------------------------------------
-- The luaparser module handles generating C source files, C header files and
-- man pages for the entire project.
--------------------------------------------------------------------------------

require(DIRNAME..'luaparser.generate_c')
require(DIRNAME..'luaparser.generate_h')
require(DIRNAME..'luaparser.generate_man')

return {
	generate_c,
	generate_h,
	generate_man,
}
