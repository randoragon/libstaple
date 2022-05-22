#include "../sp_utils.h"

/*F{*/
int sp_is_debug(void)
{
#ifdef STAPLE_DEBUG
	return 1;
#else
	return 0;
#endif
}
/*F}*/

/*F{*/
int sp_is_quiet(void)
{
#ifdef STAPLE_QUIET
	return 1;
#else
	return 0;
#endif
}
/*F}*/

/*F{*/
int sp_is_abort(void)
{
#ifdef STAPLE_ABORT
	return 1;
#else
	return 0;
#endif
}
/*F}*/

/*F{*/
#include "../internal.h"
#include <stdlib.h>
int sp_free(void *addr)
{
#ifdef STAPLE_DEBUG
	/*. C_ERR_NULLPTR addr 1 */
	/*. C_ERR_NULLPTR *(void**)addr 2 */
#endif
	free(*(void**)addr);
	return 0;
}
/*F}*/
