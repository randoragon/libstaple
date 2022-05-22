/*H{ STAPLE_UTILS_H */
/* This header defines functions that should be exposed to the library user, but
 * do not belong in just one module or data structure.
 */
/*H}*/

int sp_is_debug(void);
int sp_is_quiet(void);
int sp_is_abort(void);

int sp_free(void *addr);
