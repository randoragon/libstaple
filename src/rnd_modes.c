#include "rnd.h"

int rnd_is_debug(void)
{
#ifdef RND_DEBUG
	return 1;
#else
	return 0;
#endif
}

int rnd_is_quiet(void)
{
#ifdef RND_QUIET
	return 1;
#else
	return 0;
#endif
}

int rnd_is_abort(void)
{
#ifdef RND_ABORT
	return 1;
#else
	return 0;
#endif
}
