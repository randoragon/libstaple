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

int sp_stack_clear(struct sp_stack *stack, int (*dtor)(void*))
{
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (dtor != NULL && stack->elem_size == SP_SIZEOF_BOOL) {
		error(("destructors cannot be used on boolean elements"));
		return SP_EILLEGAL;
	}
#endif
	if (dtor != NULL) {
		const void *const end = (char*)stack->data + DATA_SIZE(stack);
		char *p = stack->data;
		while (p != end) {
			int err;
			if ((err = dtor(p))) {
				error(("callback function dtor returned %d (non-0)", err));
				return SP_ECALLBK;
			}
			p += stack->elem_size;
		}
	}
	stack->size = 0;
	return 0;
}
