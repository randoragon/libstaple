/*  Staple - A general-purpose data structure library in pure C89.
 *  Copyright (C) 2021  Randoragon
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation;
 *  version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef STAPLE_STACK_H
#define STAPLE_STACK_H

#include <stdlib.h>
#include "sp_errcodes.h"
#include "sp_modes.h"
#include "sp_utils.h"

struct sp_stack {
	void  *data;
	size_t elem_size;
	size_t size;
	size_t capacity;
};

struct sp_stack *sp_stack_create(size_t elem_size, size_t capacity);
int               sp_stack_clear(struct sp_stack *stack, int (*dtor)(void*));
int               sp_stack_destroy(struct sp_stack *stack, int (*dtor)(void*));
int               sp_stack_copy(struct sp_stack *dest, const struct sp_stack *src, int (*cpy)(void*, const void*));
int               sp_stack_foreach(struct sp_stack *stack, int (*func)(void*, size_t));

int sp_stack_push(struct sp_stack *stack, const void *elem);
int sp_stack_pushc(struct sp_stack *stack, char elem);
int sp_stack_pushs(struct sp_stack *stack, short elem);
int sp_stack_pushi(struct sp_stack *stack, int elem);
int sp_stack_pushl(struct sp_stack *stack, long elem);
int sp_stack_pushsc(struct sp_stack *stack, signed char elem);
int sp_stack_pushuc(struct sp_stack *stack, unsigned char elem);
int sp_stack_pushus(struct sp_stack *stack, unsigned short elem);
int sp_stack_pushui(struct sp_stack *stack, unsigned int elem);
int sp_stack_pushul(struct sp_stack *stack, unsigned long elem);
int sp_stack_pushf(struct sp_stack *stack, float elem);
int sp_stack_pushd(struct sp_stack *stack, double elem);
int sp_stack_pushld(struct sp_stack *stack, long double elem);
int sp_stack_pushstr(struct sp_stack *stack, const char *elem);
int sp_stack_pushstrn(struct sp_stack *stack, const char *elem, size_t len);

int sp_stack_insert(struct sp_stack *stack, size_t idx, const void *elem);
int sp_stack_insertc(struct sp_stack *stack, size_t idx, char elem);
int sp_stack_inserts(struct sp_stack *stack, size_t idx, short elem);
int sp_stack_inserti(struct sp_stack *stack, size_t idx, int elem);
int sp_stack_insertl(struct sp_stack *stack, size_t idx, long elem);
int sp_stack_insertsc(struct sp_stack *stack, size_t idx, signed char elem);
int sp_stack_insertuc(struct sp_stack *stack, size_t idx, unsigned char elem);
int sp_stack_insertus(struct sp_stack *stack, size_t idx, unsigned short elem);
int sp_stack_insertui(struct sp_stack *stack, size_t idx, unsigned int elem);
int sp_stack_insertul(struct sp_stack *stack, size_t idx, unsigned long elem);
int sp_stack_insertf(struct sp_stack *stack, size_t idx, float elem);
int sp_stack_insertd(struct sp_stack *stack, size_t idx, double elem);
int sp_stack_insertld(struct sp_stack *stack, size_t idx, long double elem);
int sp_stack_insertstr(struct sp_stack *stack, size_t idx, const char *elem);
int sp_stack_insertstrn(struct sp_stack *stack, size_t idx, const char *elem, size_t len);

int sp_stack_qinsert(struct sp_stack *stack, size_t idx, const void *elem);
int sp_stack_qinsertc(struct sp_stack *stack, size_t idx, char elem);
int sp_stack_qinserts(struct sp_stack *stack, size_t idx, short elem);
int sp_stack_qinserti(struct sp_stack *stack, size_t idx, int elem);
int sp_stack_qinsertl(struct sp_stack *stack, size_t idx, long elem);
int sp_stack_qinsertsc(struct sp_stack *stack, size_t idx, signed char elem);
int sp_stack_qinsertuc(struct sp_stack *stack, size_t idx, unsigned char elem);
int sp_stack_qinsertus(struct sp_stack *stack, size_t idx, unsigned short elem);
int sp_stack_qinsertui(struct sp_stack *stack, size_t idx, unsigned int elem);
int sp_stack_qinsertul(struct sp_stack *stack, size_t idx, unsigned long elem);
int sp_stack_qinsertf(struct sp_stack *stack, size_t idx, float elem);
int sp_stack_qinsertd(struct sp_stack *stack, size_t idx, double elem);
int sp_stack_qinsertld(struct sp_stack *stack, size_t idx, long double elem);
int sp_stack_qinsertstr(struct sp_stack *stack, size_t idx, const char *elem);
int sp_stack_qinsertstrn(struct sp_stack *stack, size_t idx, const char *elem, size_t len);

int            sp_stack_peek(const struct sp_stack *stack, void *output);
char           sp_stack_peekc(const struct sp_stack *stack);
short          sp_stack_peeks(const struct sp_stack *stack);
int            sp_stack_peeki(const struct sp_stack *stack);
long           sp_stack_peekl(const struct sp_stack *stack);
signed char    sp_stack_peeksc(const struct sp_stack *stack);
unsigned char  sp_stack_peekuc(const struct sp_stack *stack);
unsigned short sp_stack_peekus(const struct sp_stack *stack);
unsigned int   sp_stack_peekui(const struct sp_stack *stack);
unsigned long  sp_stack_peekul(const struct sp_stack *stack);
float          sp_stack_peekf(const struct sp_stack *stack);
double         sp_stack_peekd(const struct sp_stack *stack);
long double    sp_stack_peekld(const struct sp_stack *stack);
char          *sp_stack_peekstr(const struct sp_stack *stack);

int            sp_stack_pop(struct sp_stack *stack, void *output);
char           sp_stack_popc(struct sp_stack *stack);
short          sp_stack_pops(struct sp_stack *stack);
int            sp_stack_popi(struct sp_stack *stack);
long           sp_stack_popl(struct sp_stack *stack);
signed char    sp_stack_popsc(struct sp_stack *stack);
unsigned char  sp_stack_popuc(struct sp_stack *stack);
unsigned short sp_stack_popus(struct sp_stack *stack);
unsigned int   sp_stack_popui(struct sp_stack *stack);
unsigned long  sp_stack_popul(struct sp_stack *stack);
float          sp_stack_popf(struct sp_stack *stack);
double         sp_stack_popd(struct sp_stack *stack);
long double    sp_stack_popld(struct sp_stack *stack);
char          *sp_stack_popstr(struct sp_stack *stack);

int            sp_stack_remove(struct sp_stack *stack, size_t idx, void *output);
char           sp_stack_removec(struct sp_stack *stack, size_t idx);
short          sp_stack_removes(struct sp_stack *stack, size_t idx);
int            sp_stack_removei(struct sp_stack *stack, size_t idx);
long           sp_stack_removel(struct sp_stack *stack, size_t idx);
signed char    sp_stack_removesc(struct sp_stack *stack, size_t idx);
unsigned char  sp_stack_removeuc(struct sp_stack *stack, size_t idx);
unsigned short sp_stack_removeus(struct sp_stack *stack, size_t idx);
unsigned int   sp_stack_removeui(struct sp_stack *stack, size_t idx);
unsigned long  sp_stack_removeul(struct sp_stack *stack, size_t idx);
float          sp_stack_removef(struct sp_stack *stack, size_t idx);
double         sp_stack_removed(struct sp_stack *stack, size_t idx);
long double    sp_stack_removeld(struct sp_stack *stack, size_t idx);
char          *sp_stack_removestr(struct sp_stack *stack, size_t idx);

int            sp_stack_qremove(struct sp_stack *stack, size_t idx, void *output);
char           sp_stack_qremovec(struct sp_stack *stack, size_t idx);
short          sp_stack_qremoves(struct sp_stack *stack, size_t idx);
int            sp_stack_qremovei(struct sp_stack *stack, size_t idx);
long           sp_stack_qremovel(struct sp_stack *stack, size_t idx);
signed char    sp_stack_qremovesc(struct sp_stack *stack, size_t idx);
unsigned char  sp_stack_qremoveuc(struct sp_stack *stack, size_t idx);
unsigned short sp_stack_qremoveus(struct sp_stack *stack, size_t idx);
unsigned int   sp_stack_qremoveui(struct sp_stack *stack, size_t idx);
unsigned long  sp_stack_qremoveul(struct sp_stack *stack, size_t idx);
float          sp_stack_qremovef(struct sp_stack *stack, size_t idx);
double         sp_stack_qremoved(struct sp_stack *stack, size_t idx);
long double    sp_stack_qremoveld(struct sp_stack *stack, size_t idx);
char          *sp_stack_qremovestr(struct sp_stack *stack, size_t idx);

int            sp_stack_get(const struct sp_stack *stack, size_t idx, void *output);
char           sp_stack_getc(const struct sp_stack *stack, size_t idx);
short          sp_stack_gets(const struct sp_stack *stack, size_t idx);
int            sp_stack_geti(const struct sp_stack *stack, size_t idx);
long           sp_stack_getl(const struct sp_stack *stack, size_t idx);
signed char    sp_stack_getsc(const struct sp_stack *stack, size_t idx);
unsigned char  sp_stack_getuc(const struct sp_stack *stack, size_t idx);
unsigned short sp_stack_getus(const struct sp_stack *stack, size_t idx);
unsigned int   sp_stack_getui(const struct sp_stack *stack, size_t idx);
unsigned long  sp_stack_getul(const struct sp_stack *stack, size_t idx);
float          sp_stack_getf(const struct sp_stack *stack, size_t idx);
double         sp_stack_getd(const struct sp_stack *stack, size_t idx);
long double    sp_stack_getld(const struct sp_stack *stack, size_t idx);
char          *sp_stack_getstr(const struct sp_stack *stack, size_t idx);

int sp_stack_set(struct sp_stack *stack, size_t idx, void *val);
int sp_stack_setc(struct sp_stack *stack, size_t idx, char val);
int sp_stack_sets(struct sp_stack *stack, size_t idx, short val);
int sp_stack_seti(struct sp_stack *stack, size_t idx, int val);
int sp_stack_setl(struct sp_stack *stack, size_t idx, long val);
int sp_stack_setsc(struct sp_stack *stack, size_t idx, signed char val);
int sp_stack_setuc(struct sp_stack *stack, size_t idx, unsigned char val);
int sp_stack_setus(struct sp_stack *stack, size_t idx, unsigned short val);
int sp_stack_setui(struct sp_stack *stack, size_t idx, unsigned int val);
int sp_stack_setul(struct sp_stack *stack, size_t idx, unsigned long val);
int sp_stack_setf(struct sp_stack *stack, size_t idx, float val);
int sp_stack_setd(struct sp_stack *stack, size_t idx, double val);
int sp_stack_setld(struct sp_stack *stack, size_t idx, long double val);
int sp_stack_setstr(struct sp_stack *stack, size_t idx, const char *val);
int sp_stack_setstrn(struct sp_stack *stack, size_t idx, const char *val, size_t len);

int sp_stack_print(const struct sp_stack *stack);
int sp_stack_printc(const struct sp_stack *stack);
int sp_stack_prints(const struct sp_stack *stack);
int sp_stack_printi(const struct sp_stack *stack);
int sp_stack_printl(const struct sp_stack *stack);
int sp_stack_printsc(const struct sp_stack *stack);
int sp_stack_printuc(const struct sp_stack *stack);
int sp_stack_printus(const struct sp_stack *stack);
int sp_stack_printui(const struct sp_stack *stack);
int sp_stack_printul(const struct sp_stack *stack);
int sp_stack_printf(const struct sp_stack *stack);
int sp_stack_printd(const struct sp_stack *stack);
int sp_stack_printld(const struct sp_stack *stack);
int sp_stack_printstr(const struct sp_stack *stack);

#endif /* STAPLE_STACK_H */
