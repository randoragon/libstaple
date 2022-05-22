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
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#include "../sp_stack.h"
#include "../internal.h"
#include <stdint.h>
#include <inttypes.h>

uint32_t sp_stack_qremoveu32(struct sp_stack *stack, size_t idx)
{
	char *p, *q;
	uint32_t ret;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return 0;
	}
	if (stack->elem_size != sizeof(uint32_t)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(uint32_t)));
		return 0;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return 0;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	q = (char*)stack->data + (stack->size - 1) * stack->elem_size;
	ret = *(uint32_t*)p;
	*(uint32_t*)p = *(uint32_t*)q;
	--stack->size;
	return ret;
}

#else
typedef int prevent_empty_translation_unit;
#endif