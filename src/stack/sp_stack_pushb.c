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
#include "../sp_stack.h"
#include "../internal.h"
#include "../sp_utils.h"
#include "../sp_errcodes.h"

int sp_stack_pushb(struct sp_stack *stack, int elem)
{
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != SP_SIZEOF_BOOL) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, SP_SIZEOF_BOOL));
		return SP_EILLEGAL;
	}
#endif
	if (stack->size % SP_BYTE_SIZE == 0) {
		if (sp_size_try_add(stack->size, 1))
			return SP_ERANGE;
		if (sp_boolbuf_fit(&stack->data, stack->size, &stack->capacity))
			return SP_ENOMEM;
	}
	sp_boolbuf_set(stack->size++, elem, stack->data);
	return 0;
}
