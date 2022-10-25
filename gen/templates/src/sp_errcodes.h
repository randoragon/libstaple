/*H{ STAPLE_ERRCODES_H */
/* This header defines symbolic names for all error codes returned by various
 * functions of the library. That way it's easier for the end-user to capture,
 * identify and handle errors.
 */
/*H}*/

/* Invalid argmuent value */
#define SP_EINVAL 1

/* Insufficient memory */
#define SP_ENOMEM 2

/* External function handler returned error */
#define SP_EHANDLER 3

/* Index out of range */
#define SP_EINDEX 4

/* Illegal operation */
#define SP_EILLEGAL 5

/* Numerical range exceeded (underflow/overflow) */
#define SP_ERANGE 6
