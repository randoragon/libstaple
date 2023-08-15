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

/* The stack module of the staple library. */

#include <stdlib.h>
#include "sp_errcodes.h"
#include "sp_utils.h"
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#include <stdint.h>
#endif

struct sp_stack {
	void  *data;
	size_t elem_size;
	size_t size;
	size_t capacity;
};

struct sp_stack *sp_stack_create(size_t elem_size, size_t capacity);
int              sp_stack_clear(struct sp_stack *stack, int (*dtor)(void*));
int              sp_stack_destroy(struct sp_stack *stack, int (*dtor)(void*));
int              sp_stack_eq(const struct sp_stack *stack1, const struct sp_stack *stack2, int (*cmp)(const void*, const void*));
int              sp_stack_copy(struct sp_stack *dest, const struct sp_stack *src, int (*cpy)(void*, const void*));
int              sp_stack_map(struct sp_stack *stack, int (*func)(void*, size_t));

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
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
int sp_stack_pushll(struct sp_stack *stack, long long elem);
int sp_stack_pushull(struct sp_stack *stack, unsigned long long elem);
int sp_stack_pushu8(struct sp_stack *stack, uint8_t elem);
int sp_stack_pushu16(struct sp_stack *stack, uint16_t elem);
int sp_stack_pushu32(struct sp_stack *stack, uint32_t elem);
int sp_stack_pushu64(struct sp_stack *stack, uint64_t elem);
int sp_stack_pushi8(struct sp_stack *stack, int8_t elem);
int sp_stack_pushi16(struct sp_stack *stack, int16_t elem);
int sp_stack_pushi32(struct sp_stack *stack, int32_t elem);
int sp_stack_pushi64(struct sp_stack *stack, int64_t elem);
#endif
int sp_stack_pushb(struct sp_stack *stack, int elem);
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
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
int sp_stack_insertll(struct sp_stack *stack, size_t idx, long long elem);
int sp_stack_insertull(struct sp_stack *stack, size_t idx, unsigned long long elem);
int sp_stack_insertu8(struct sp_stack *stack, size_t idx, uint8_t elem);
int sp_stack_insertu16(struct sp_stack *stack, size_t idx, uint16_t elem);
int sp_stack_insertu32(struct sp_stack *stack, size_t idx, uint32_t elem);
int sp_stack_insertu64(struct sp_stack *stack, size_t idx, uint64_t elem);
int sp_stack_inserti8(struct sp_stack *stack, size_t idx, int8_t elem);
int sp_stack_inserti16(struct sp_stack *stack, size_t idx, int16_t elem);
int sp_stack_inserti32(struct sp_stack *stack, size_t idx, int32_t elem);
int sp_stack_inserti64(struct sp_stack *stack, size_t idx, int64_t elem);
#endif
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
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
int sp_stack_qinsertll(struct sp_stack *stack, size_t idx, long long elem);
int sp_stack_qinsertull(struct sp_stack *stack, size_t idx, unsigned long long elem);
int sp_stack_qinsertu8(struct sp_stack *stack, size_t idx, uint8_t elem);
int sp_stack_qinsertu16(struct sp_stack *stack, size_t idx, uint16_t elem);
int sp_stack_qinsertu32(struct sp_stack *stack, size_t idx, uint32_t elem);
int sp_stack_qinsertu64(struct sp_stack *stack, size_t idx, uint64_t elem);
int sp_stack_qinserti8(struct sp_stack *stack, size_t idx, int8_t elem);
int sp_stack_qinserti16(struct sp_stack *stack, size_t idx, int16_t elem);
int sp_stack_qinserti32(struct sp_stack *stack, size_t idx, int32_t elem);
int sp_stack_qinserti64(struct sp_stack *stack, size_t idx, int64_t elem);
#endif
int sp_stack_qinsertstr(struct sp_stack *stack, size_t idx, const char *elem);
int sp_stack_qinsertstrn(struct sp_stack *stack, size_t idx, const char *elem, size_t len);

void   *sp_stack_peek(const struct sp_stack *stack);
char  sp_stack_peekc(const struct sp_stack *stack);
short  sp_stack_peeks(const struct sp_stack *stack);
int  sp_stack_peeki(const struct sp_stack *stack);
long  sp_stack_peekl(const struct sp_stack *stack);
signed char  sp_stack_peeksc(const struct sp_stack *stack);
unsigned char  sp_stack_peekuc(const struct sp_stack *stack);
unsigned short  sp_stack_peekus(const struct sp_stack *stack);
unsigned int  sp_stack_peekui(const struct sp_stack *stack);
unsigned long  sp_stack_peekul(const struct sp_stack *stack);
float  sp_stack_peekf(const struct sp_stack *stack);
double  sp_stack_peekd(const struct sp_stack *stack);
long double  sp_stack_peekld(const struct sp_stack *stack);
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
long long  sp_stack_peekll(const struct sp_stack *stack);
unsigned long long  sp_stack_peekull(const struct sp_stack *stack);
uint8_t  sp_stack_peeku8(const struct sp_stack *stack);
uint16_t  sp_stack_peeku16(const struct sp_stack *stack);
uint32_t  sp_stack_peeku32(const struct sp_stack *stack);
uint64_t  sp_stack_peeku64(const struct sp_stack *stack);
int8_t  sp_stack_peeki8(const struct sp_stack *stack);
int16_t  sp_stack_peeki16(const struct sp_stack *stack);
int32_t  sp_stack_peeki32(const struct sp_stack *stack);
int64_t  sp_stack_peeki64(const struct sp_stack *stack);
#endif
int     sp_stack_peekb(const struct sp_stack *stack);
char   *sp_stack_peekstr(const struct sp_stack *stack);

int     sp_stack_pop(struct sp_stack *stack, int (*dtor)(void*));
char  sp_stack_popc(struct sp_stack *stack);
short  sp_stack_pops(struct sp_stack *stack);
int  sp_stack_popi(struct sp_stack *stack);
long  sp_stack_popl(struct sp_stack *stack);
signed char  sp_stack_popsc(struct sp_stack *stack);
unsigned char  sp_stack_popuc(struct sp_stack *stack);
unsigned short  sp_stack_popus(struct sp_stack *stack);
unsigned int  sp_stack_popui(struct sp_stack *stack);
unsigned long  sp_stack_popul(struct sp_stack *stack);
float  sp_stack_popf(struct sp_stack *stack);
double  sp_stack_popd(struct sp_stack *stack);
long double  sp_stack_popld(struct sp_stack *stack);
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
long long  sp_stack_popll(struct sp_stack *stack);
unsigned long long  sp_stack_popull(struct sp_stack *stack);
uint8_t  sp_stack_popu8(struct sp_stack *stack);
uint16_t  sp_stack_popu16(struct sp_stack *stack);
uint32_t  sp_stack_popu32(struct sp_stack *stack);
uint64_t  sp_stack_popu64(struct sp_stack *stack);
int8_t  sp_stack_popi8(struct sp_stack *stack);
int16_t  sp_stack_popi16(struct sp_stack *stack);
int32_t  sp_stack_popi32(struct sp_stack *stack);
int64_t  sp_stack_popi64(struct sp_stack *stack);
#endif
char   *sp_stack_popstr(struct sp_stack *stack);

int     sp_stack_remove(struct sp_stack *stack, size_t idx, int (*dtor)(void*));
char  sp_stack_removec(struct sp_stack *stack, size_t idx);
short  sp_stack_removes(struct sp_stack *stack, size_t idx);
int  sp_stack_removei(struct sp_stack *stack, size_t idx);
long  sp_stack_removel(struct sp_stack *stack, size_t idx);
signed char  sp_stack_removesc(struct sp_stack *stack, size_t idx);
unsigned char  sp_stack_removeuc(struct sp_stack *stack, size_t idx);
unsigned short  sp_stack_removeus(struct sp_stack *stack, size_t idx);
unsigned int  sp_stack_removeui(struct sp_stack *stack, size_t idx);
unsigned long  sp_stack_removeul(struct sp_stack *stack, size_t idx);
float  sp_stack_removef(struct sp_stack *stack, size_t idx);
double  sp_stack_removed(struct sp_stack *stack, size_t idx);
long double  sp_stack_removeld(struct sp_stack *stack, size_t idx);
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
long long  sp_stack_removell(struct sp_stack *stack, size_t idx);
unsigned long long  sp_stack_removeull(struct sp_stack *stack, size_t idx);
uint8_t  sp_stack_removeu8(struct sp_stack *stack, size_t idx);
uint16_t  sp_stack_removeu16(struct sp_stack *stack, size_t idx);
uint32_t  sp_stack_removeu32(struct sp_stack *stack, size_t idx);
uint64_t  sp_stack_removeu64(struct sp_stack *stack, size_t idx);
int8_t  sp_stack_removei8(struct sp_stack *stack, size_t idx);
int16_t  sp_stack_removei16(struct sp_stack *stack, size_t idx);
int32_t  sp_stack_removei32(struct sp_stack *stack, size_t idx);
int64_t  sp_stack_removei64(struct sp_stack *stack, size_t idx);
#endif
char   *sp_stack_removestr(struct sp_stack *stack, size_t idx);

int     sp_stack_qremove(struct sp_stack *stack, size_t idx, int (*dtor)(void*));
char  sp_stack_qremovec(struct sp_stack *stack, size_t idx);
short  sp_stack_qremoves(struct sp_stack *stack, size_t idx);
int  sp_stack_qremovei(struct sp_stack *stack, size_t idx);
long  sp_stack_qremovel(struct sp_stack *stack, size_t idx);
signed char  sp_stack_qremovesc(struct sp_stack *stack, size_t idx);
unsigned char  sp_stack_qremoveuc(struct sp_stack *stack, size_t idx);
unsigned short  sp_stack_qremoveus(struct sp_stack *stack, size_t idx);
unsigned int  sp_stack_qremoveui(struct sp_stack *stack, size_t idx);
unsigned long  sp_stack_qremoveul(struct sp_stack *stack, size_t idx);
float  sp_stack_qremovef(struct sp_stack *stack, size_t idx);
double  sp_stack_qremoved(struct sp_stack *stack, size_t idx);
long double  sp_stack_qremoveld(struct sp_stack *stack, size_t idx);
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
long long  sp_stack_qremovell(struct sp_stack *stack, size_t idx);
unsigned long long  sp_stack_qremoveull(struct sp_stack *stack, size_t idx);
uint8_t  sp_stack_qremoveu8(struct sp_stack *stack, size_t idx);
uint16_t  sp_stack_qremoveu16(struct sp_stack *stack, size_t idx);
uint32_t  sp_stack_qremoveu32(struct sp_stack *stack, size_t idx);
uint64_t  sp_stack_qremoveu64(struct sp_stack *stack, size_t idx);
int8_t  sp_stack_qremovei8(struct sp_stack *stack, size_t idx);
int16_t  sp_stack_qremovei16(struct sp_stack *stack, size_t idx);
int32_t  sp_stack_qremovei32(struct sp_stack *stack, size_t idx);
int64_t  sp_stack_qremovei64(struct sp_stack *stack, size_t idx);
#endif
char   *sp_stack_qremovestr(struct sp_stack *stack, size_t idx);

void   *sp_stack_get(const struct sp_stack *stack, size_t idx);
char  sp_stack_getc(const struct sp_stack *stack, size_t idx);
short  sp_stack_gets(const struct sp_stack *stack, size_t idx);
int  sp_stack_geti(const struct sp_stack *stack, size_t idx);
long  sp_stack_getl(const struct sp_stack *stack, size_t idx);
signed char  sp_stack_getsc(const struct sp_stack *stack, size_t idx);
unsigned char  sp_stack_getuc(const struct sp_stack *stack, size_t idx);
unsigned short  sp_stack_getus(const struct sp_stack *stack, size_t idx);
unsigned int  sp_stack_getui(const struct sp_stack *stack, size_t idx);
unsigned long  sp_stack_getul(const struct sp_stack *stack, size_t idx);
float  sp_stack_getf(const struct sp_stack *stack, size_t idx);
double  sp_stack_getd(const struct sp_stack *stack, size_t idx);
long double  sp_stack_getld(const struct sp_stack *stack, size_t idx);
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
long long  sp_stack_getll(const struct sp_stack *stack, size_t idx);
unsigned long long  sp_stack_getull(const struct sp_stack *stack, size_t idx);
uint8_t  sp_stack_getu8(const struct sp_stack *stack, size_t idx);
uint16_t  sp_stack_getu16(const struct sp_stack *stack, size_t idx);
uint32_t  sp_stack_getu32(const struct sp_stack *stack, size_t idx);
uint64_t  sp_stack_getu64(const struct sp_stack *stack, size_t idx);
int8_t  sp_stack_geti8(const struct sp_stack *stack, size_t idx);
int16_t  sp_stack_geti16(const struct sp_stack *stack, size_t idx);
int32_t  sp_stack_geti32(const struct sp_stack *stack, size_t idx);
int64_t  sp_stack_geti64(const struct sp_stack *stack, size_t idx);
#endif
int     sp_stack_getb(const struct sp_stack *stack, size_t idx);
char   *sp_stack_getstr(const struct sp_stack *stack, size_t idx);

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
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
int sp_stack_setll(struct sp_stack *stack, size_t idx, long long val);
int sp_stack_setull(struct sp_stack *stack, size_t idx, unsigned long long val);
int sp_stack_setu8(struct sp_stack *stack, size_t idx, uint8_t val);
int sp_stack_setu16(struct sp_stack *stack, size_t idx, uint16_t val);
int sp_stack_setu32(struct sp_stack *stack, size_t idx, uint32_t val);
int sp_stack_setu64(struct sp_stack *stack, size_t idx, uint64_t val);
int sp_stack_seti8(struct sp_stack *stack, size_t idx, int8_t val);
int sp_stack_seti16(struct sp_stack *stack, size_t idx, int16_t val);
int sp_stack_seti32(struct sp_stack *stack, size_t idx, int32_t val);
int sp_stack_seti64(struct sp_stack *stack, size_t idx, int64_t val);
#endif
int sp_stack_setb(struct sp_stack *stack, size_t idx, int val);
int sp_stack_setstr(struct sp_stack *stack, size_t idx, const char *val);
int sp_stack_setstrn(struct sp_stack *stack, size_t idx, const char *val, size_t len);

int sp_stack_print(const struct sp_stack *stack, int (*func)(const void*));
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
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
int sp_stack_printll(const struct sp_stack *stack);
int sp_stack_printull(const struct sp_stack *stack);
int sp_stack_printu8(const struct sp_stack *stack);
int sp_stack_printu16(const struct sp_stack *stack);
int sp_stack_printu32(const struct sp_stack *stack);
int sp_stack_printu64(const struct sp_stack *stack);
int sp_stack_printi8(const struct sp_stack *stack);
int sp_stack_printi16(const struct sp_stack *stack);
int sp_stack_printi32(const struct sp_stack *stack);
int sp_stack_printi64(const struct sp_stack *stack);
#endif
int sp_stack_printstr(const struct sp_stack *stack);

#endif /* STAPLE_STACK_H */
