#ifndef RND_QUEUE_H
#define RND_QUEUE_H

#include <stdlib.h>

struct rnd_queue {
	void *data;
	void *head;
	void *tail;
	size_t elem_size;
	size_t size;
	size_t capacity;
};

struct rnd_queue *rnd_queue_create(size_t elem_size, size_t capacity);
int               rnd_queue_clear(struct rnd_queue *queue, int (*dtor)(void*));
int               rnd_queue_destroy(struct rnd_queue *queue, int (*dtor)(void*));
int               rnd_queue_copy(struct rnd_queue *dest, const struct rnd_queue *src, int (*cpy)(void*, const void*));
int               rnd_queue_map(struct rnd_queue *queue, int (*func)(void*, size_t));

int rnd_queue_push(struct rnd_queue *queue, const void *elem);
int rnd_queue_pushc(struct rnd_queue *queue, char elem);
int rnd_queue_pushs(struct rnd_queue *queue, short elem);
int rnd_queue_pushi(struct rnd_queue *queue, int elem);
int rnd_queue_pushl(struct rnd_queue *queue, long elem);
int rnd_queue_pushsc(struct rnd_queue *queue, signed char elem);
int rnd_queue_pushuc(struct rnd_queue *queue, unsigned char elem);
int rnd_queue_pushus(struct rnd_queue *queue, unsigned short elem);
int rnd_queue_pushui(struct rnd_queue *queue, unsigned int elem);
int rnd_queue_pushul(struct rnd_queue *queue, unsigned long elem);
int rnd_queue_pushf(struct rnd_queue *queue, float elem);
int rnd_queue_pushd(struct rnd_queue *queue, double elem);
int rnd_queue_pushld(struct rnd_queue *queue, long double elem);

int rnd_queue_insert(struct rnd_queue *queue, size_t idx, const void *elem);
int rnd_queue_insertc(struct rnd_queue *queue, size_t idx, char elem);
int rnd_queue_inserts(struct rnd_queue *queue, size_t idx, short elem);
int rnd_queue_inserti(struct rnd_queue *queue, size_t idx, int elem);
int rnd_queue_insertl(struct rnd_queue *queue, size_t idx, long elem);
int rnd_queue_insertsc(struct rnd_queue *queue, size_t idx, signed char elem);
int rnd_queue_insertuc(struct rnd_queue *queue, size_t idx, unsigned char elem);
int rnd_queue_insertus(struct rnd_queue *queue, size_t idx, unsigned short elem);
int rnd_queue_insertui(struct rnd_queue *queue, size_t idx, unsigned int elem);
int rnd_queue_insertul(struct rnd_queue *queue, size_t idx, unsigned long elem);
int rnd_queue_insertf(struct rnd_queue *queue, size_t idx, float elem);
int rnd_queue_insertd(struct rnd_queue *queue, size_t idx, double elem);
int rnd_queue_insertld(struct rnd_queue *queue, size_t idx, long double elem);

int rnd_queue_quickinsert(struct rnd_queue *queue, size_t idx, const void *elem);
int rnd_queue_quickinsertc(struct rnd_queue *queue, size_t idx, char elem);
int rnd_queue_quickinserts(struct rnd_queue *queue, size_t idx, short elem);
int rnd_queue_quickinserti(struct rnd_queue *queue, size_t idx, int elem);
int rnd_queue_quickinsertl(struct rnd_queue *queue, size_t idx, long elem);
int rnd_queue_quickinsertsc(struct rnd_queue *queue, size_t idx, signed char elem);
int rnd_queue_quickinsertuc(struct rnd_queue *queue, size_t idx, unsigned char elem);
int rnd_queue_quickinsertus(struct rnd_queue *queue, size_t idx, unsigned short elem);
int rnd_queue_quickinsertui(struct rnd_queue *queue, size_t idx, unsigned int elem);
int rnd_queue_quickinsertul(struct rnd_queue *queue, size_t idx, unsigned long elem);
int rnd_queue_quickinsertf(struct rnd_queue *queue, size_t idx, float elem);
int rnd_queue_quickinsertd(struct rnd_queue *queue, size_t idx, double elem);
int rnd_queue_quickinsertld(struct rnd_queue *queue, size_t idx, long double elem);

int            rnd_queue_peek(const struct rnd_queue *queue, void *output);
char           rnd_queue_peekc(const struct rnd_queue *queue);
short          rnd_queue_peeks(const struct rnd_queue *queue);
int            rnd_queue_peeki(const struct rnd_queue *queue);
long           rnd_queue_peekl(const struct rnd_queue *queue);
signed char    rnd_queue_peeksc(const struct rnd_queue *queue);
unsigned char  rnd_queue_peekuc(const struct rnd_queue *queue);
unsigned short rnd_queue_peekus(const struct rnd_queue *queue);
unsigned int   rnd_queue_peekui(const struct rnd_queue *queue);
unsigned long  rnd_queue_peekul(const struct rnd_queue *queue);
float          rnd_queue_peekf(const struct rnd_queue *queue);
double         rnd_queue_peekd(const struct rnd_queue *queue);
long double    rnd_queue_peekld(const struct rnd_queue *queue);

int            rnd_queue_pop(struct rnd_queue *queue, void *output);
char           rnd_queue_popc(struct rnd_queue *queue);
short          rnd_queue_pops(struct rnd_queue *queue);
int            rnd_queue_popi(struct rnd_queue *queue);
long           rnd_queue_popl(struct rnd_queue *queue);
signed char    rnd_queue_popsc(struct rnd_queue *queue);
unsigned char  rnd_queue_popuc(struct rnd_queue *queue);
unsigned short rnd_queue_popus(struct rnd_queue *queue);
unsigned int   rnd_queue_popui(struct rnd_queue *queue);
unsigned long  rnd_queue_popul(struct rnd_queue *queue);
float          rnd_queue_popf(struct rnd_queue *queue);
double         rnd_queue_popd(struct rnd_queue *queue);
long double    rnd_queue_popld(struct rnd_queue *queue);

int            rnd_queue_remove(struct rnd_queue *queue, size_t idx, void *output);
char           rnd_queue_removec(struct rnd_queue *queue, size_t idx);
short          rnd_queue_removes(struct rnd_queue *queue, size_t idx);
int            rnd_queue_removei(struct rnd_queue *queue, size_t idx);
long           rnd_queue_removel(struct rnd_queue *queue, size_t idx);
signed char    rnd_queue_removesc(struct rnd_queue *queue, size_t idx);
unsigned char  rnd_queue_removeuc(struct rnd_queue *queue, size_t idx);
unsigned short rnd_queue_removeus(struct rnd_queue *queue, size_t idx);
unsigned int   rnd_queue_removeui(struct rnd_queue *queue, size_t idx);
unsigned long  rnd_queue_removeul(struct rnd_queue *queue, size_t idx);
float          rnd_queue_removef(struct rnd_queue *queue, size_t idx);
double         rnd_queue_removed(struct rnd_queue *queue, size_t idx);
long double    rnd_queue_removeld(struct rnd_queue *queue, size_t idx);

int            rnd_queue_quickremove(struct rnd_queue *queue, size_t idx, void *output);
char           rnd_queue_quickremovec(struct rnd_queue *queue, size_t idx);
short          rnd_queue_quickremoves(struct rnd_queue *queue, size_t idx);
int            rnd_queue_quickremovei(struct rnd_queue *queue, size_t idx);
long           rnd_queue_quickremovel(struct rnd_queue *queue, size_t idx);
signed char    rnd_queue_quickremovesc(struct rnd_queue *queue, size_t idx);
unsigned char  rnd_queue_quickremoveuc(struct rnd_queue *queue, size_t idx);
unsigned short rnd_queue_quickremoveus(struct rnd_queue *queue, size_t idx);
unsigned int   rnd_queue_quickremoveui(struct rnd_queue *queue, size_t idx);
unsigned long  rnd_queue_quickremoveul(struct rnd_queue *queue, size_t idx);
float          rnd_queue_quickremovef(struct rnd_queue *queue, size_t idx);
double         rnd_queue_quickremoved(struct rnd_queue *queue, size_t idx);
long double    rnd_queue_quickremoveld(struct rnd_queue *queue, size_t idx);

int            rnd_queue_get(const struct rnd_queue *queue, size_t idx, void *output);
char           rnd_queue_getc(const struct rnd_queue *queue, size_t idx);
short          rnd_queue_gets(const struct rnd_queue *queue, size_t idx);
int            rnd_queue_geti(const struct rnd_queue *queue, size_t idx);
long           rnd_queue_getl(const struct rnd_queue *queue, size_t idx);
signed char    rnd_queue_getsc(const struct rnd_queue *queue, size_t idx);
unsigned char  rnd_queue_getuc(const struct rnd_queue *queue, size_t idx);
unsigned short rnd_queue_getus(const struct rnd_queue *queue, size_t idx);
unsigned int   rnd_queue_getui(const struct rnd_queue *queue, size_t idx);
unsigned long  rnd_queue_getul(const struct rnd_queue *queue, size_t idx);
float          rnd_queue_getf(const struct rnd_queue *queue, size_t idx);
double         rnd_queue_getd(const struct rnd_queue *queue, size_t idx);
long double    rnd_queue_getld(const struct rnd_queue *queue, size_t idx);

int rnd_queue_set(struct rnd_queue *queue, size_t idx, void *val);
int rnd_queue_setc(struct rnd_queue *queue, size_t idx, char val);
int rnd_queue_sets(struct rnd_queue *queue, size_t idx, short val);
int rnd_queue_seti(struct rnd_queue *queue, size_t idx, int val);
int rnd_queue_setl(struct rnd_queue *queue, size_t idx, long val);
int rnd_queue_setsc(struct rnd_queue *queue, size_t idx, signed char val);
int rnd_queue_setuc(struct rnd_queue *queue, size_t idx, unsigned char val);
int rnd_queue_setus(struct rnd_queue *queue, size_t idx, unsigned short val);
int rnd_queue_setui(struct rnd_queue *queue, size_t idx, unsigned int val);
int rnd_queue_setul(struct rnd_queue *queue, size_t idx, unsigned long val);
int rnd_queue_setf(struct rnd_queue *queue, size_t idx, float val);
int rnd_queue_setd(struct rnd_queue *queue, size_t idx, double val);
int rnd_queue_setld(struct rnd_queue *queue, size_t idx, long double val);

int rnd_queue_print(struct rnd_queue *queue);
int rnd_queue_printc(struct rnd_queue *queue);
int rnd_queue_prints(struct rnd_queue *queue);
int rnd_queue_printi(struct rnd_queue *queue);
int rnd_queue_printl(struct rnd_queue *queue);
int rnd_queue_printsc(struct rnd_queue *queue);
int rnd_queue_printuc(struct rnd_queue *queue);
int rnd_queue_printus(struct rnd_queue *queue);
int rnd_queue_printui(struct rnd_queue *queue);
int rnd_queue_printul(struct rnd_queue *queue);
int rnd_queue_printf(struct rnd_queue *queue);
int rnd_queue_printd(struct rnd_queue *queue);
int rnd_queue_printld(struct rnd_queue *queue);

#endif /* RND_QUEUE_H */
