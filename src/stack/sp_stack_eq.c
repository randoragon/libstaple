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
#include <string.h>

int sp_stack_eq(const struct sp_stack *stack1, const struct sp_stack *stack2, int (*cmp)(const void*, const void*))
{
#ifdef STAPLE_DEBUG
	if (stack1 == NULL) {
		error(("stack1 is NULL"));
		return 0;
	}
	if (stack2 == NULL) {
		error(("stack2 is NULL"));
		return 0;
	}
#endif
	if (stack1->elem_size != stack2->elem_size || stack1->size != stack2->size)
		return 0;
	if (cmp) {
		size_t i;
		for (i = 0; i < stack1->size; i++) {
			const void *const p = (char*)stack1->data + i * stack1->elem_size,
			           *const q = (char*)stack2->data + i * stack2->elem_size;
			if (cmp(p, q))
				return 0;
		}
		return 1;
	}
	return !memcmp(stack1->data, stack2->data, DATA_SIZE(stack1));
}
