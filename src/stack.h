#ifndef RND_STACK_H
#define RND_STACK_H

#include <stdlib.h>

struct rnd_stack {
	void  *data;
	size_t elem_size;
	size_t size;
	size_t capacity;
};

struct rnd_stack *rnd_stack_create(size_t elem_size, size_t capacity);
int               rnd_stack_clear(struct rnd_stack *stack, int (*dtor)(void*));
int               rnd_stack_destroy(struct rnd_stack *stack, int (*dtor)(void*));
int               rnd_stack_copy(struct rnd_stack *dest, const struct rnd_stack *src, void *(*cpy)(const void *));

int rnd_stack_pushp(struct rnd_stack *stack, const void *elem);
int rnd_stack_pushc(struct rnd_stack *stack, char elem);
int rnd_stack_pushs(struct rnd_stack *stack, short elem);
int rnd_stack_pushi(struct rnd_stack *stack, int elem);
int rnd_stack_pushl(struct rnd_stack *stack, long elem);
int rnd_stack_pushuc(struct rnd_stack *stack, unsigned char elem);
int rnd_stack_pushus(struct rnd_stack *stack, unsigned short elem);
int rnd_stack_pushui(struct rnd_stack *stack, unsigned int elem);
int rnd_stack_pushul(struct rnd_stack *stack, unsigned long elem);
int rnd_stack_pushf(struct rnd_stack *stack, float elem);
int rnd_stack_pushd(struct rnd_stack *stack, double elem);
int rnd_stack_pushld(struct rnd_stack *stack, long double elem);

int rnd_stack_insertp(struct rnd_stack *stack, size_t idx, const void *elem);
int rnd_stack_insertc(struct rnd_stack *stack, size_t idx, char elem);
int rnd_stack_inserts(struct rnd_stack *stack, size_t idx, short elem);
int rnd_stack_inserti(struct rnd_stack *stack, size_t idx, int elem);
int rnd_stack_insertl(struct rnd_stack *stack, size_t idx, long elem);
int rnd_stack_insertuc(struct rnd_stack *stack, size_t idx, unsigned char elem);
int rnd_stack_insertus(struct rnd_stack *stack, size_t idx, unsigned short elem);
int rnd_stack_insertui(struct rnd_stack *stack, size_t idx, unsigned int elem);
int rnd_stack_insertul(struct rnd_stack *stack, size_t idx, unsigned long elem);
int rnd_stack_insertf(struct rnd_stack *stack, size_t idx, float elem);
int rnd_stack_insertd(struct rnd_stack *stack, size_t idx, double elem);
int rnd_stack_insertld(struct rnd_stack *stack, size_t idx, long double elem);

void          *rnd_stack_peekp(const struct rnd_stack *stack);
char           rnd_stack_peekc(const struct rnd_stack *stack);
short          rnd_stack_peeks(const struct rnd_stack *stack);
int            rnd_stack_peeki(const struct rnd_stack *stack);
long           rnd_stack_peekl(const struct rnd_stack *stack);
unsigned char  rnd_stack_peekuc(const struct rnd_stack *stack);
unsigned short rnd_stack_peekus(const struct rnd_stack *stack);
unsigned int   rnd_stack_peekui(const struct rnd_stack *stack);
unsigned long  rnd_stack_peekul(const struct rnd_stack *stack);
float          rnd_stack_peekf(const struct rnd_stack *stack);
double         rnd_stack_peekd(const struct rnd_stack *stack);
long double    rnd_stack_peekld(const struct rnd_stack *stack);

void          *rnd_stack_popp(struct rnd_stack *stack);
char           rnd_stack_popc(struct rnd_stack *stack);
short          rnd_stack_pops(struct rnd_stack *stack);
int            rnd_stack_popi(struct rnd_stack *stack);
long           rnd_stack_popl(struct rnd_stack *stack);
unsigned char  rnd_stack_popuc(struct rnd_stack *stack);
unsigned short rnd_stack_popus(struct rnd_stack *stack);
unsigned int   rnd_stack_popui(struct rnd_stack *stack);
unsigned long  rnd_stack_popul(struct rnd_stack *stack);
float          rnd_stack_popf(struct rnd_stack *stack);
double         rnd_stack_popd(struct rnd_stack *stack);
long double    rnd_stack_popld(struct rnd_stack *stack);

void          *rnd_stack_removep(struct rnd_stack *stack, size_t idx);
char           rnd_stack_removec(struct rnd_stack *stack, size_t idx);
short          rnd_stack_removes(struct rnd_stack *stack, size_t idx);
int            rnd_stack_removei(struct rnd_stack *stack, size_t idx);
long           rnd_stack_removel(struct rnd_stack *stack, size_t idx);
unsigned char  rnd_stack_removeuc(struct rnd_stack *stack, size_t idx);
unsigned short rnd_stack_removeus(struct rnd_stack *stack, size_t idx);
unsigned int   rnd_stack_removeui(struct rnd_stack *stack, size_t idx);
unsigned long  rnd_stack_removeul(struct rnd_stack *stack, size_t idx);
float          rnd_stack_removef(struct rnd_stack *stack, size_t idx);
double         rnd_stack_removed(struct rnd_stack *stack, size_t idx);
long double    rnd_stack_removeld(struct rnd_stack *stack, size_t idx);

#endif /* RND_STACK_H */
