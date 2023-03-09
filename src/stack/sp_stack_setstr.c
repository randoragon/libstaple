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

int sp_stack_setstr(struct sp_stack *stack, size_t idx, const char *val)
{
	char *p;
	char *buf;
	size_t len;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (val == NULL) {
		error(("val is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(val)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(val)));
		return SP_EILLEGAL;
	}
	if (idx >= stack->size) {
		error(("index out of range"));
		return SP_EINDEX;
	}
#endif
	p = (char*)stack->data + (stack->size - 1 - idx) * stack->elem_size;
	free(*(char**)p);
	len = sp_strnlen(val, SP_SIZE_MAX);
	if (sp_size_try_add(len, 1))
		return SP_ERANGE;
	buf = malloc((len + 1) * sizeof(*val));
	if (buf == NULL) {
		error(("malloc"));
		return SP_ENOMEM;
	}
	memcpy(buf, val, len * sizeof(*val));
	buf[len] = '\0';
	*(char**)p = buf;
	return 0;
}
