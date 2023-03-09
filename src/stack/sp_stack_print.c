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
#include "../sp_errcodes.h"

int sp_stack_print(const struct sp_stack *stack, int (*func)(const void*))
{
	size_t i;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
#endif
	printf("sp_stack_print()\nsize/capacity: %lu/%lu, elem_size: %lu\n",
		(unsigned long)stack->size, (unsigned long)stack->capacity, (unsigned long)stack->elem_size);
	if (func == NULL)
		for (i = stack->size; i-- > 0;) {
			const void *const elem = (char*)stack->data + i * stack->elem_size;
			printf("[%lu]\t%p\n", (unsigned long)stack->size - 1 - i, elem);
		}
	else
		for (i = stack->size; i-- > 0;) {
			const void *const elem = (char*)stack->data + i * stack->elem_size;
			int err;
			printf("[%lu]\t", (unsigned long)stack->size - 1 - i);
			if ((err = func(elem)) != 0) {
				error(("callback function dtor returned %d (non-0)", err));
				return SP_ECALLBK;
			}
		}
	return 0;
}
