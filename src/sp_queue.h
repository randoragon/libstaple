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
#ifndef STAPLE_QUEUE_H
#define STAPLE_QUEUE_H

/* The queue module of the staple library. */

#include <stdlib.h>
#include "sp_errcodes.h"
#include "sp_utils.h"
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#include <stdint.h>
#endif

struct sp_queue {
	void *data;
	void *head;
	void *tail;
	size_t elem_size;
	size_t size;
	size_t capacity;
};

struct sp_queue *sp_queue_create(size_t elem_size, size_t capacity);
int              sp_queue_clear(struct sp_queue *queue, int (*dtor)(void*));
int              sp_queue_destroy(struct sp_queue *queue, int (*dtor)(void*));
int              sp_queue_eq(const struct sp_queue *queue1, const struct sp_queue *queue2, int (*cmp)(const void*, const void*));
int              sp_queue_copy(struct sp_queue *dest, const struct sp_queue *src, int (*cpy)(void*, const void*));
int              sp_queue_map(struct sp_queue *queue, int (*func)(void*, size_t));

int sp_queue_push(struct sp_queue *queue, const void *elem);
int sp_queue_pushc(struct sp_queue *queue, char elem);
int sp_queue_pushs(struct sp_queue *queue, short elem);
int sp_queue_pushi(struct sp_queue *queue, int elem);
int sp_queue_pushl(struct sp_queue *queue, long elem);
int sp_queue_pushsc(struct sp_queue *queue, signed char elem);
int sp_queue_pushuc(struct sp_queue *queue, unsigned char elem);
int sp_queue_pushus(struct sp_queue *queue, unsigned short elem);
int sp_queue_pushui(struct sp_queue *queue, unsigned int elem);
int sp_queue_pushul(struct sp_queue *queue, unsigned long elem);
int sp_queue_pushf(struct sp_queue *queue, float elem);
int sp_queue_pushd(struct sp_queue *queue, double elem);
int sp_queue_pushld(struct sp_queue *queue, long double elem);
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
int sp_queue_pushll(struct sp_queue *queue, long long elem);
int sp_queue_pushull(struct sp_queue *queue, unsigned long long elem);
int sp_queue_pushu8(struct sp_queue *queue, uint8_t elem);
int sp_queue_pushu16(struct sp_queue *queue, uint16_t elem);
int sp_queue_pushu32(struct sp_queue *queue, uint32_t elem);
int sp_queue_pushu64(struct sp_queue *queue, uint64_t elem);
int sp_queue_pushi8(struct sp_queue *queue, int8_t elem);
int sp_queue_pushi16(struct sp_queue *queue, int16_t elem);
int sp_queue_pushi32(struct sp_queue *queue, int32_t elem);
int sp_queue_pushi64(struct sp_queue *queue, int64_t elem);
#endif
int sp_queue_pushstr(struct sp_queue *queue, const char *elem);
int sp_queue_pushstrn(struct sp_queue *queue, const char *elem, size_t len);

int sp_queue_insert(struct sp_queue *queue, size_t idx, const void *elem);
int sp_queue_insertc(struct sp_queue *queue, size_t idx, char elem);
int sp_queue_inserts(struct sp_queue *queue, size_t idx, short elem);
int sp_queue_inserti(struct sp_queue *queue, size_t idx, int elem);
int sp_queue_insertl(struct sp_queue *queue, size_t idx, long elem);
int sp_queue_insertsc(struct sp_queue *queue, size_t idx, signed char elem);
int sp_queue_insertuc(struct sp_queue *queue, size_t idx, unsigned char elem);
int sp_queue_insertus(struct sp_queue *queue, size_t idx, unsigned short elem);
int sp_queue_insertui(struct sp_queue *queue, size_t idx, unsigned int elem);
int sp_queue_insertul(struct sp_queue *queue, size_t idx, unsigned long elem);
int sp_queue_insertf(struct sp_queue *queue, size_t idx, float elem);
int sp_queue_insertd(struct sp_queue *queue, size_t idx, double elem);
int sp_queue_insertld(struct sp_queue *queue, size_t idx, long double elem);
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
int sp_queue_insertll(struct sp_queue *queue, size_t idx, long long elem);
int sp_queue_insertull(struct sp_queue *queue, size_t idx, unsigned long long elem);
int sp_queue_insertu8(struct sp_queue *queue, size_t idx, uint8_t elem);
int sp_queue_insertu16(struct sp_queue *queue, size_t idx, uint16_t elem);
int sp_queue_insertu32(struct sp_queue *queue, size_t idx, uint32_t elem);
int sp_queue_insertu64(struct sp_queue *queue, size_t idx, uint64_t elem);
int sp_queue_inserti8(struct sp_queue *queue, size_t idx, int8_t elem);
int sp_queue_inserti16(struct sp_queue *queue, size_t idx, int16_t elem);
int sp_queue_inserti32(struct sp_queue *queue, size_t idx, int32_t elem);
int sp_queue_inserti64(struct sp_queue *queue, size_t idx, int64_t elem);
#endif
int sp_queue_insertstr(struct sp_queue *queue, size_t idx, const char *elem);
int sp_queue_insertstrn(struct sp_queue *queue, size_t idx, const char *elem, size_t len);

int sp_queue_qinsert(struct sp_queue *queue, size_t idx, const void *elem);
int sp_queue_qinsertc(struct sp_queue *queue, size_t idx, char elem);
int sp_queue_qinserts(struct sp_queue *queue, size_t idx, short elem);
int sp_queue_qinserti(struct sp_queue *queue, size_t idx, int elem);
int sp_queue_qinsertl(struct sp_queue *queue, size_t idx, long elem);
int sp_queue_qinsertsc(struct sp_queue *queue, size_t idx, signed char elem);
int sp_queue_qinsertuc(struct sp_queue *queue, size_t idx, unsigned char elem);
int sp_queue_qinsertus(struct sp_queue *queue, size_t idx, unsigned short elem);
int sp_queue_qinsertui(struct sp_queue *queue, size_t idx, unsigned int elem);
int sp_queue_qinsertul(struct sp_queue *queue, size_t idx, unsigned long elem);
int sp_queue_qinsertf(struct sp_queue *queue, size_t idx, float elem);
int sp_queue_qinsertd(struct sp_queue *queue, size_t idx, double elem);
int sp_queue_qinsertld(struct sp_queue *queue, size_t idx, long double elem);
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
int sp_queue_qinsertll(struct sp_queue *queue, size_t idx, long long elem);
int sp_queue_qinsertull(struct sp_queue *queue, size_t idx, unsigned long long elem);
int sp_queue_qinsertu8(struct sp_queue *queue, size_t idx, uint8_t elem);
int sp_queue_qinsertu16(struct sp_queue *queue, size_t idx, uint16_t elem);
int sp_queue_qinsertu32(struct sp_queue *queue, size_t idx, uint32_t elem);
int sp_queue_qinsertu64(struct sp_queue *queue, size_t idx, uint64_t elem);
int sp_queue_qinserti8(struct sp_queue *queue, size_t idx, int8_t elem);
int sp_queue_qinserti16(struct sp_queue *queue, size_t idx, int16_t elem);
int sp_queue_qinserti32(struct sp_queue *queue, size_t idx, int32_t elem);
int sp_queue_qinserti64(struct sp_queue *queue, size_t idx, int64_t elem);
#endif
int sp_queue_qinsertstr(struct sp_queue *queue, size_t idx, const char *elem);
int sp_queue_qinsertstrn(struct sp_queue *queue, size_t idx, const char *elem, size_t len);

void          *sp_queue_peek(const struct sp_queue *queue);
char         sp_queue_peekc(const struct sp_queue *queue);
short         sp_queue_peeks(const struct sp_queue *queue);
int         sp_queue_peeki(const struct sp_queue *queue);
long         sp_queue_peekl(const struct sp_queue *queue);
signed char         sp_queue_peeksc(const struct sp_queue *queue);
unsigned char         sp_queue_peekuc(const struct sp_queue *queue);
unsigned short         sp_queue_peekus(const struct sp_queue *queue);
unsigned int         sp_queue_peekui(const struct sp_queue *queue);
unsigned long         sp_queue_peekul(const struct sp_queue *queue);
float         sp_queue_peekf(const struct sp_queue *queue);
double         sp_queue_peekd(const struct sp_queue *queue);
long double         sp_queue_peekld(const struct sp_queue *queue);
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
long long         sp_queue_peekll(const struct sp_queue *queue);
unsigned long long         sp_queue_peekull(const struct sp_queue *queue);
uint8_t         sp_queue_peeku8(const struct sp_queue *queue);
uint16_t         sp_queue_peeku16(const struct sp_queue *queue);
uint32_t         sp_queue_peeku32(const struct sp_queue *queue);
uint64_t         sp_queue_peeku64(const struct sp_queue *queue);
int8_t         sp_queue_peeki8(const struct sp_queue *queue);
int16_t         sp_queue_peeki16(const struct sp_queue *queue);
int32_t         sp_queue_peeki32(const struct sp_queue *queue);
int64_t         sp_queue_peeki64(const struct sp_queue *queue);
#endif
char          *sp_queue_peekstr(const struct sp_queue *queue);

int            sp_queue_pop(struct sp_queue *queue, int (*dtor)(void*));
char         sp_queue_popc(struct sp_queue *queue);
short         sp_queue_pops(struct sp_queue *queue);
int         sp_queue_popi(struct sp_queue *queue);
long         sp_queue_popl(struct sp_queue *queue);
signed char         sp_queue_popsc(struct sp_queue *queue);
unsigned char         sp_queue_popuc(struct sp_queue *queue);
unsigned short         sp_queue_popus(struct sp_queue *queue);
unsigned int         sp_queue_popui(struct sp_queue *queue);
unsigned long         sp_queue_popul(struct sp_queue *queue);
float         sp_queue_popf(struct sp_queue *queue);
double         sp_queue_popd(struct sp_queue *queue);
long double         sp_queue_popld(struct sp_queue *queue);
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
long long         sp_queue_popll(struct sp_queue *queue);
unsigned long long         sp_queue_popull(struct sp_queue *queue);
uint8_t         sp_queue_popu8(struct sp_queue *queue);
uint16_t         sp_queue_popu16(struct sp_queue *queue);
uint32_t         sp_queue_popu32(struct sp_queue *queue);
uint64_t         sp_queue_popu64(struct sp_queue *queue);
int8_t         sp_queue_popi8(struct sp_queue *queue);
int16_t         sp_queue_popi16(struct sp_queue *queue);
int32_t         sp_queue_popi32(struct sp_queue *queue);
int64_t         sp_queue_popi64(struct sp_queue *queue);
#endif
char          *sp_queue_popstr(struct sp_queue *queue);

int            sp_queue_remove(struct sp_queue *queue, size_t idx, int (*dtor)(void*));
char         sp_queue_removec(struct sp_queue *queue, size_t idx);
short         sp_queue_removes(struct sp_queue *queue, size_t idx);
int         sp_queue_removei(struct sp_queue *queue, size_t idx);
long         sp_queue_removel(struct sp_queue *queue, size_t idx);
signed char         sp_queue_removesc(struct sp_queue *queue, size_t idx);
unsigned char         sp_queue_removeuc(struct sp_queue *queue, size_t idx);
unsigned short         sp_queue_removeus(struct sp_queue *queue, size_t idx);
unsigned int         sp_queue_removeui(struct sp_queue *queue, size_t idx);
unsigned long         sp_queue_removeul(struct sp_queue *queue, size_t idx);
float         sp_queue_removef(struct sp_queue *queue, size_t idx);
double         sp_queue_removed(struct sp_queue *queue, size_t idx);
long double         sp_queue_removeld(struct sp_queue *queue, size_t idx);
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
long long         sp_queue_removell(struct sp_queue *queue, size_t idx);
unsigned long long         sp_queue_removeull(struct sp_queue *queue, size_t idx);
uint8_t         sp_queue_removeu8(struct sp_queue *queue, size_t idx);
uint16_t         sp_queue_removeu16(struct sp_queue *queue, size_t idx);
uint32_t         sp_queue_removeu32(struct sp_queue *queue, size_t idx);
uint64_t         sp_queue_removeu64(struct sp_queue *queue, size_t idx);
int8_t         sp_queue_removei8(struct sp_queue *queue, size_t idx);
int16_t         sp_queue_removei16(struct sp_queue *queue, size_t idx);
int32_t         sp_queue_removei32(struct sp_queue *queue, size_t idx);
int64_t         sp_queue_removei64(struct sp_queue *queue, size_t idx);
#endif
char          *sp_queue_removestr(struct sp_queue *queue, size_t idx);

int            sp_queue_qremove(struct sp_queue *queue, size_t idx, int (*dtor)(void*));
char         sp_queue_qremovec(struct sp_queue *queue, size_t idx);
short         sp_queue_qremoves(struct sp_queue *queue, size_t idx);
int         sp_queue_qremovei(struct sp_queue *queue, size_t idx);
long         sp_queue_qremovel(struct sp_queue *queue, size_t idx);
signed char         sp_queue_qremovesc(struct sp_queue *queue, size_t idx);
unsigned char         sp_queue_qremoveuc(struct sp_queue *queue, size_t idx);
unsigned short         sp_queue_qremoveus(struct sp_queue *queue, size_t idx);
unsigned int         sp_queue_qremoveui(struct sp_queue *queue, size_t idx);
unsigned long         sp_queue_qremoveul(struct sp_queue *queue, size_t idx);
float         sp_queue_qremovef(struct sp_queue *queue, size_t idx);
double         sp_queue_qremoved(struct sp_queue *queue, size_t idx);
long double         sp_queue_qremoveld(struct sp_queue *queue, size_t idx);
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
long long         sp_queue_qremovell(struct sp_queue *queue, size_t idx);
unsigned long long         sp_queue_qremoveull(struct sp_queue *queue, size_t idx);
uint8_t         sp_queue_qremoveu8(struct sp_queue *queue, size_t idx);
uint16_t         sp_queue_qremoveu16(struct sp_queue *queue, size_t idx);
uint32_t         sp_queue_qremoveu32(struct sp_queue *queue, size_t idx);
uint64_t         sp_queue_qremoveu64(struct sp_queue *queue, size_t idx);
int8_t         sp_queue_qremovei8(struct sp_queue *queue, size_t idx);
int16_t         sp_queue_qremovei16(struct sp_queue *queue, size_t idx);
int32_t         sp_queue_qremovei32(struct sp_queue *queue, size_t idx);
int64_t         sp_queue_qremovei64(struct sp_queue *queue, size_t idx);
#endif
char          *sp_queue_qremovestr(struct sp_queue *queue, size_t idx);

void          *sp_queue_get(const struct sp_queue *queue, size_t idx);
char         sp_queue_getc(const struct sp_queue *queue, size_t idx);
short         sp_queue_gets(const struct sp_queue *queue, size_t idx);
int         sp_queue_geti(const struct sp_queue *queue, size_t idx);
long         sp_queue_getl(const struct sp_queue *queue, size_t idx);
signed char         sp_queue_getsc(const struct sp_queue *queue, size_t idx);
unsigned char         sp_queue_getuc(const struct sp_queue *queue, size_t idx);
unsigned short         sp_queue_getus(const struct sp_queue *queue, size_t idx);
unsigned int         sp_queue_getui(const struct sp_queue *queue, size_t idx);
unsigned long         sp_queue_getul(const struct sp_queue *queue, size_t idx);
float         sp_queue_getf(const struct sp_queue *queue, size_t idx);
double         sp_queue_getd(const struct sp_queue *queue, size_t idx);
long double         sp_queue_getld(const struct sp_queue *queue, size_t idx);
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
long long         sp_queue_getll(const struct sp_queue *queue, size_t idx);
unsigned long long         sp_queue_getull(const struct sp_queue *queue, size_t idx);
uint8_t         sp_queue_getu8(const struct sp_queue *queue, size_t idx);
uint16_t         sp_queue_getu16(const struct sp_queue *queue, size_t idx);
uint32_t         sp_queue_getu32(const struct sp_queue *queue, size_t idx);
uint64_t         sp_queue_getu64(const struct sp_queue *queue, size_t idx);
int8_t         sp_queue_geti8(const struct sp_queue *queue, size_t idx);
int16_t         sp_queue_geti16(const struct sp_queue *queue, size_t idx);
int32_t         sp_queue_geti32(const struct sp_queue *queue, size_t idx);
int64_t         sp_queue_geti64(const struct sp_queue *queue, size_t idx);
#endif
char          *sp_queue_getstr(const struct sp_queue *queue, size_t idx);

int sp_queue_set(struct sp_queue *queue, size_t idx, void *val);
int sp_queue_setc(struct sp_queue *queue, size_t idx, char val);
int sp_queue_sets(struct sp_queue *queue, size_t idx, short val);
int sp_queue_seti(struct sp_queue *queue, size_t idx, int val);
int sp_queue_setl(struct sp_queue *queue, size_t idx, long val);
int sp_queue_setsc(struct sp_queue *queue, size_t idx, signed char val);
int sp_queue_setuc(struct sp_queue *queue, size_t idx, unsigned char val);
int sp_queue_setus(struct sp_queue *queue, size_t idx, unsigned short val);
int sp_queue_setui(struct sp_queue *queue, size_t idx, unsigned int val);
int sp_queue_setul(struct sp_queue *queue, size_t idx, unsigned long val);
int sp_queue_setf(struct sp_queue *queue, size_t idx, float val);
int sp_queue_setd(struct sp_queue *queue, size_t idx, double val);
int sp_queue_setld(struct sp_queue *queue, size_t idx, long double val);
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
int sp_queue_setll(struct sp_queue *queue, size_t idx, long long val);
int sp_queue_setull(struct sp_queue *queue, size_t idx, unsigned long long val);
int sp_queue_setu8(struct sp_queue *queue, size_t idx, uint8_t val);
int sp_queue_setu16(struct sp_queue *queue, size_t idx, uint16_t val);
int sp_queue_setu32(struct sp_queue *queue, size_t idx, uint32_t val);
int sp_queue_setu64(struct sp_queue *queue, size_t idx, uint64_t val);
int sp_queue_seti8(struct sp_queue *queue, size_t idx, int8_t val);
int sp_queue_seti16(struct sp_queue *queue, size_t idx, int16_t val);
int sp_queue_seti32(struct sp_queue *queue, size_t idx, int32_t val);
int sp_queue_seti64(struct sp_queue *queue, size_t idx, int64_t val);
#endif
int sp_queue_setstr(struct sp_queue *queue, size_t idx, const char *val);
int sp_queue_setstrn(struct sp_queue *queue, size_t idx, const char *val, size_t len);

int sp_queue_print(const struct sp_queue *queue, int (*func)(const void*));
int sp_queue_printc(const struct sp_queue *queue);
int sp_queue_prints(const struct sp_queue *queue);
int sp_queue_printi(const struct sp_queue *queue);
int sp_queue_printl(const struct sp_queue *queue);
int sp_queue_printsc(const struct sp_queue *queue);
int sp_queue_printuc(const struct sp_queue *queue);
int sp_queue_printus(const struct sp_queue *queue);
int sp_queue_printui(const struct sp_queue *queue);
int sp_queue_printul(const struct sp_queue *queue);
int sp_queue_printf(const struct sp_queue *queue);
int sp_queue_printd(const struct sp_queue *queue);
int sp_queue_printld(const struct sp_queue *queue);
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
int sp_queue_printll(const struct sp_queue *queue);
int sp_queue_printull(const struct sp_queue *queue);
int sp_queue_printu8(const struct sp_queue *queue);
int sp_queue_printu16(const struct sp_queue *queue);
int sp_queue_printu32(const struct sp_queue *queue);
int sp_queue_printu64(const struct sp_queue *queue);
int sp_queue_printi8(const struct sp_queue *queue);
int sp_queue_printi16(const struct sp_queue *queue);
int sp_queue_printi32(const struct sp_queue *queue);
int sp_queue_printi64(const struct sp_queue *queue);
#endif
int sp_queue_printstr(const struct sp_queue *queue);

#endif /* STAPLE_QUEUE_H */
