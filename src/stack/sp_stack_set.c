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
#include <string.h>

int sp_stack_set(struct sp_stack *stack, size_t idx, void *val)
{
	char *p;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (val == NULL) {
		error(("val is NULL"));
		return SP_EINVAL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	memcpy(p, val, stack->elem_size);
	return 0;
}