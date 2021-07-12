/* This header defines symbolic names for all error codes returned by various
 * functions of the library. That way it's easier for the end-user to capture,
 * identify and handle errors.
 */

/* Invalid argmuent value */
#define RND_EINVAL 1

/* Insufficient memory */
#define RND_ENOMEM 2

/* External function handler returned error */
#define RND_EHANDLER 3

/* Index out of range */
#define RND_EINDEX 4

/* Illegal operation */
#define RND_EILLEGAL 5

/* Numerical range exceeded (underflow/overflow) */
#define RND_ERANGE 6
