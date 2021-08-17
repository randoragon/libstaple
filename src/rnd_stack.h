#ifndef RND_STACK_H
#define RND_STACK_H

#include <stdlib.h>
#include "rnd_errcodes.h"

struct rnd_stack {
	void  *data;
	size_t elem_size;
	size_t size;
	size_t capacity;
};

struct rnd_stack *rnd_stack_create(size_t elem_size, size_t capacity);
int               rnd_stack_clear(struct rnd_stack *stack, int (*dtor)(void*));
int               rnd_stack_destroy(struct rnd_stack *stack, int (*dtor)(void*));
int               rnd_stack_copy(struct rnd_stack *dest, const struct rnd_stack *src, int (*cpy)(void*, const void*));
int               rnd_stack_foreach(struct rnd_stack *stack, int (*func)(void*, size_t));

int rnd_stack_push(struct rnd_stack *stack, const void *elem);
int rnd_stack_pushc(struct rnd_stack *stack, char elem);
int rnd_stack_pushs(struct rnd_stack *stack, short elem);
int rnd_stack_pushi(struct rnd_stack *stack, int elem);
int rnd_stack_pushl(struct rnd_stack *stack, long elem);
int rnd_stack_pushsc(struct rnd_stack *stack, signed char elem);
int rnd_stack_pushuc(struct rnd_stack *stack, unsigned char elem);
int rnd_stack_pushus(struct rnd_stack *stack, unsigned short elem);
int rnd_stack_pushui(struct rnd_stack *stack, unsigned int elem);
int rnd_stack_pushul(struct rnd_stack *stack, unsigned long elem);
int rnd_stack_pushf(struct rnd_stack *stack, float elem);
int rnd_stack_pushd(struct rnd_stack *stack, double elem);
int rnd_stack_pushld(struct rnd_stack *stack, long double elem);

int rnd_stack_insert(struct rnd_stack *stack, size_t idx, const void *elem);
int rnd_stack_insertc(struct rnd_stack *stack, size_t idx, char elem);
int rnd_stack_inserts(struct rnd_stack *stack, size_t idx, short elem);
int rnd_stack_inserti(struct rnd_stack *stack, size_t idx, int elem);
int rnd_stack_insertl(struct rnd_stack *stack, size_t idx, long elem);
int rnd_stack_insertsc(struct rnd_stack *stack, size_t idx, signed char elem);
int rnd_stack_insertuc(struct rnd_stack *stack, size_t idx, unsigned char elem);
int rnd_stack_insertus(struct rnd_stack *stack, size_t idx, unsigned short elem);
int rnd_stack_insertui(struct rnd_stack *stack, size_t idx, unsigned int elem);
int rnd_stack_insertul(struct rnd_stack *stack, size_t idx, unsigned long elem);
int rnd_stack_insertf(struct rnd_stack *stack, size_t idx, float elem);
int rnd_stack_insertd(struct rnd_stack *stack, size_t idx, double elem);
int rnd_stack_insertld(struct rnd_stack *stack, size_t idx, long double elem);

int rnd_stack_qinsert(struct rnd_stack *stack, size_t idx, const void *elem);
int rnd_stack_qinsertc(struct rnd_stack *stack, size_t idx, char elem);
int rnd_stack_qinserts(struct rnd_stack *stack, size_t idx, short elem);
int rnd_stack_qinserti(struct rnd_stack *stack, size_t idx, int elem);
int rnd_stack_qinsertl(struct rnd_stack *stack, size_t idx, long elem);
int rnd_stack_qinsertsc(struct rnd_stack *stack, size_t idx, signed char elem);
int rnd_stack_qinsertuc(struct rnd_stack *stack, size_t idx, unsigned char elem);
int rnd_stack_qinsertus(struct rnd_stack *stack, size_t idx, unsigned short elem);
int rnd_stack_qinsertui(struct rnd_stack *stack, size_t idx, unsigned int elem);
int rnd_stack_qinsertul(struct rnd_stack *stack, size_t idx, unsigned long elem);
int rnd_stack_qinsertf(struct rnd_stack *stack, size_t idx, float elem);
int rnd_stack_qinsertd(struct rnd_stack *stack, size_t idx, double elem);
int rnd_stack_qinsertld(struct rnd_stack *stack, size_t idx, long double elem);

int            rnd_stack_peek(const struct rnd_stack *stack, void *output);
char           rnd_stack_peekc(const struct rnd_stack *stack);
short          rnd_stack_peeks(const struct rnd_stack *stack);
int            rnd_stack_peeki(const struct rnd_stack *stack);
long           rnd_stack_peekl(const struct rnd_stack *stack);
signed char    rnd_stack_peeksc(const struct rnd_stack *stack);
unsigned char  rnd_stack_peekuc(const struct rnd_stack *stack);
unsigned short rnd_stack_peekus(const struct rnd_stack *stack);
unsigned int   rnd_stack_peekui(const struct rnd_stack *stack);
unsigned long  rnd_stack_peekul(const struct rnd_stack *stack);
float          rnd_stack_peekf(const struct rnd_stack *stack);
double         rnd_stack_peekd(const struct rnd_stack *stack);
long double    rnd_stack_peekld(const struct rnd_stack *stack);

int            rnd_stack_pop(struct rnd_stack *stack, void *output);
char           rnd_stack_popc(struct rnd_stack *stack);
short          rnd_stack_pops(struct rnd_stack *stack);
int            rnd_stack_popi(struct rnd_stack *stack);
long           rnd_stack_popl(struct rnd_stack *stack);
signed char    rnd_stack_popsc(struct rnd_stack *stack);
unsigned char  rnd_stack_popuc(struct rnd_stack *stack);
unsigned short rnd_stack_popus(struct rnd_stack *stack);
unsigned int   rnd_stack_popui(struct rnd_stack *stack);
unsigned long  rnd_stack_popul(struct rnd_stack *stack);
float          rnd_stack_popf(struct rnd_stack *stack);
double         rnd_stack_popd(struct rnd_stack *stack);
long double    rnd_stack_popld(struct rnd_stack *stack);

int            rnd_stack_remove(struct rnd_stack *stack, size_t idx, void *output);
char           rnd_stack_removec(struct rnd_stack *stack, size_t idx);
short          rnd_stack_removes(struct rnd_stack *stack, size_t idx);
int            rnd_stack_removei(struct rnd_stack *stack, size_t idx);
long           rnd_stack_removel(struct rnd_stack *stack, size_t idx);
signed char    rnd_stack_removesc(struct rnd_stack *stack, size_t idx);
unsigned char  rnd_stack_removeuc(struct rnd_stack *stack, size_t idx);
unsigned short rnd_stack_removeus(struct rnd_stack *stack, size_t idx);
unsigned int   rnd_stack_removeui(struct rnd_stack *stack, size_t idx);
unsigned long  rnd_stack_removeul(struct rnd_stack *stack, size_t idx);
float          rnd_stack_removef(struct rnd_stack *stack, size_t idx);
double         rnd_stack_removed(struct rnd_stack *stack, size_t idx);
long double    rnd_stack_removeld(struct rnd_stack *stack, size_t idx);

int            rnd_stack_qremove(struct rnd_stack *stack, size_t idx, void *output);
char           rnd_stack_qremovec(struct rnd_stack *stack, size_t idx);
short          rnd_stack_qremoves(struct rnd_stack *stack, size_t idx);
int            rnd_stack_qremovei(struct rnd_stack *stack, size_t idx);
long           rnd_stack_qremovel(struct rnd_stack *stack, size_t idx);
signed char    rnd_stack_qremovesc(struct rnd_stack *stack, size_t idx);
unsigned char  rnd_stack_qremoveuc(struct rnd_stack *stack, size_t idx);
unsigned short rnd_stack_qremoveus(struct rnd_stack *stack, size_t idx);
unsigned int   rnd_stack_qremoveui(struct rnd_stack *stack, size_t idx);
unsigned long  rnd_stack_qremoveul(struct rnd_stack *stack, size_t idx);
float          rnd_stack_qremovef(struct rnd_stack *stack, size_t idx);
double         rnd_stack_qremoved(struct rnd_stack *stack, size_t idx);
long double    rnd_stack_qremoveld(struct rnd_stack *stack, size_t idx);

int            rnd_stack_get(const struct rnd_stack *stack, size_t idx, void *output);
char           rnd_stack_getc(const struct rnd_stack *stack, size_t idx);
short          rnd_stack_gets(const struct rnd_stack *stack, size_t idx);
int            rnd_stack_geti(const struct rnd_stack *stack, size_t idx);
long           rnd_stack_getl(const struct rnd_stack *stack, size_t idx);
signed char    rnd_stack_getsc(const struct rnd_stack *stack, size_t idx);
unsigned char  rnd_stack_getuc(const struct rnd_stack *stack, size_t idx);
unsigned short rnd_stack_getus(const struct rnd_stack *stack, size_t idx);
unsigned int   rnd_stack_getui(const struct rnd_stack *stack, size_t idx);
unsigned long  rnd_stack_getul(const struct rnd_stack *stack, size_t idx);
float          rnd_stack_getf(const struct rnd_stack *stack, size_t idx);
double         rnd_stack_getd(const struct rnd_stack *stack, size_t idx);
long double    rnd_stack_getld(const struct rnd_stack *stack, size_t idx);

int rnd_stack_set(struct rnd_stack *stack, size_t idx, void *val);
int rnd_stack_setc(struct rnd_stack *stack, size_t idx, char val);
int rnd_stack_sets(struct rnd_stack *stack, size_t idx, short val);
int rnd_stack_seti(struct rnd_stack *stack, size_t idx, int val);
int rnd_stack_setl(struct rnd_stack *stack, size_t idx, long val);
int rnd_stack_setsc(struct rnd_stack *stack, size_t idx, signed char val);
int rnd_stack_setuc(struct rnd_stack *stack, size_t idx, unsigned char val);
int rnd_stack_setus(struct rnd_stack *stack, size_t idx, unsigned short val);
int rnd_stack_setui(struct rnd_stack *stack, size_t idx, unsigned int val);
int rnd_stack_setul(struct rnd_stack *stack, size_t idx, unsigned long val);
int rnd_stack_setf(struct rnd_stack *stack, size_t idx, float val);
int rnd_stack_setd(struct rnd_stack *stack, size_t idx, double val);
int rnd_stack_setld(struct rnd_stack *stack, size_t idx, long double val);

int rnd_stack_print(struct rnd_stack *stack);
int rnd_stack_printc(struct rnd_stack *stack);
int rnd_stack_prints(struct rnd_stack *stack);
int rnd_stack_printi(struct rnd_stack *stack);
int rnd_stack_printl(struct rnd_stack *stack);
int rnd_stack_printsc(struct rnd_stack *stack);
int rnd_stack_printuc(struct rnd_stack *stack);
int rnd_stack_printus(struct rnd_stack *stack);
int rnd_stack_printui(struct rnd_stack *stack);
int rnd_stack_printul(struct rnd_stack *stack);
int rnd_stack_printf(struct rnd_stack *stack);
int rnd_stack_printd(struct rnd_stack *stack);
int rnd_stack_printld(struct rnd_stack *stack);

#endif /* RND_STACK_H */
