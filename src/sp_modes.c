#include "staple.h"

int sp_is_debug(void)
{
#ifdef STAPLE_DEBUG
	return 1;
#else
	return 0;
#endif
}

int sp_is_quiet(void)
{
#ifdef STAPLE_QUIET
	return 1;
#else
	return 0;
#endif
}

int sp_is_abort(void)
{
#ifdef STAPLE_ABORT
	return 1;
#else
	return 0;
#endif
}
