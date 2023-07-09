-- This program generates the C source files from a rudimentary template format.
-- This was necessary, because the C preprocessor does not offer support for
-- advanced file splitting.

DIRNAME = arg[0]:match('(.*/)') or ''
dofile(DIRNAME..'pathconf.lua')

require(DIRNAME..'luaparser')

-- GLOBAL CONFIGURATION
BASEDIR         = DIRNAME..'../'
SNIPPETDIR      = DIRNAME..'snippets/'
TEMPLATEDIR     = DIRNAME..'templates/'
C_HEADER_TEXT   = io.open(SNIPPETDIR..'C_LICENSE_HEADER', 'r'):read('a')
MAN_HEADER_TEXT = io.open(SNIPPETDIR..'MAN_LICENSE_HEADER', 'r'):read('a')

-- You can define the suffixed functions to generate here
local pconf = ParamConfig:new{
	C89 = {
		{ TYPE='char'          , SUFFIX='c' , FMT=[["%hd\t'%c'"]] },
		{ TYPE='short'         , SUFFIX='s' , FMT=[["%hd"]]       },
		{ TYPE='int'           , SUFFIX='i' , FMT=[["%d"]]        },
		{ TYPE='long'          , SUFFIX='l' , FMT=[["%ld"]]       },
		{ TYPE='signed char'   , SUFFIX='sc', FMT=[["%hd\t'%c'"]] },
		{ TYPE='unsigned char' , SUFFIX='uc', FMT=[["%hd\t'%c'"]] },
		{ TYPE='unsigned short', SUFFIX='us', FMT=[["%hu"]]       },
		{ TYPE='unsigned int'  , SUFFIX='ui', FMT=[["%u"]]        },
		{ TYPE='unsigned long' , SUFFIX='ul', FMT=[["%lu"]]       },
		{ TYPE='float'         , SUFFIX='f' , FMT=[["%g"]]        },
		{ TYPE='double'        , SUFFIX='d' , FMT=[["%g"]]        },
		{ TYPE='long double'   , SUFFIX='ld', FMT=[["%Lg"]]       },
		-- bool, str and strn suffixed functions are implemented separately
	},

	C99 = {
		INCLUDE = { 'stdint.h', 'inttypes.h' },
		{ TYPE='long long'         , SUFFIX='ll' , FMT=[["%lld"]]    },
		{ TYPE='unsigned long long', SUFFIX='ull', FMT=[["%llu"]]    },
		{ TYPE='uint8_t'           , SUFFIX='u8' , FMT=[["%"PRIu8]]  },
		{ TYPE='uint16_t'          , SUFFIX='u16', FMT=[["%"PRIu16]] },
		{ TYPE='uint32_t'          , SUFFIX='u32', FMT=[["%"PRIu32]] },
		{ TYPE='uint64_t'          , SUFFIX='u64', FMT=[["%"PRIu64]] },
		{ TYPE='int8_t'            , SUFFIX='i8' , FMT=[["%"PRId8]]  },
		{ TYPE='int16_t'           , SUFFIX='i16', FMT=[["%"PRId16]] },
		{ TYPE='int32_t'           , SUFFIX='i32', FMT=[["%"PRId32]] },
		{ TYPE='int64_t'           , SUFFIX='i64', FMT=[["%"PRId64]] },
	}
}

-- Generate source files for every template
for _, t in templates_iter(C_TEMPLATES) do
	local dirname = t:match('(.*)/')
	generate_c(BASEDIR..dirname, TEMPLATEDIR..t, pconf)
end
for _, t in templates_iter(H_TEMPLATES) do
	local dirname = t:match('(.*)/')
	generate_h(BASEDIR..dirname, TEMPLATEDIR..t, pconf)
end
for _, t in templates_iter(MAN_TEMPLATES) do
	local dirname = t:match('(.*)/')
	generate_man(BASEDIR..dirname, TEMPLATEDIR..t, pconf)
end
