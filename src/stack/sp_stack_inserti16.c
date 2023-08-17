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
#include "../sp_errcodes.h"
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

int sp_stack_inserti16(struct sp_stack *stack, size_t idx, int16_t elem)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(elem)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(elem)));
		return SP_EILLEGAL;
	}
	if (idx > stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	if (sp_size_try_add(DATA_SIZE(stack), stack->elem_size))
		return SP_ERANGE;
	if (sp_buf_fit(&stack->data, stack->size, &stack->capacity, stack->elem_size))
		return SP_ENOMEM;
	p = (char*)stack->data + (stack->size - idx) * stack->elem_size;
	memmove(p + stack->elem_size, p, idx * stack->elem_size);
	*(int16_t*)p = elem;
	++stack->size;
	return 0;
}

#else
typedef int prevent_empty_translation_unit;
#endif
