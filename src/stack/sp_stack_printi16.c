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
#include <stdint.h>
#include <inttypes.h>

int sp_stack_printi16(const struct sp_stack *stack)
{
	size_t i;
#ifdef STAPLE_DEBUG
	if (stack == NULL) {
		error(("stack is NULL"));
		return SP_EINVAL;
	}
	if (stack->elem_size != sizeof(int16_t)) {
		error(("stack->elem_size is incompatible with elem type (%lu != %lu)",
					(unsigned long)stack->elem_size, sizeof(int16_t)));
		return SP_EILLEGAL;
	}
#endif
	printf("sp_stack_printi16()\nsize/capacity: "SP_SIZE_FMT"/"SP_SIZE_FMT", elem_size: "SP_SIZE_FMT"\n",
		(SP_SIZE_T)stack->size, (SP_SIZE_T)stack->capacity, (SP_SIZE_T)stack->elem_size);
	for (i = stack->size; i-- > 0;) {
		const int16_t elem = ((int16_t*)stack->data)[i];
		printf("["SP_SIZE_FMT"]\t""%"PRId16"\n", (SP_SIZE_T)stack->size - 1 - i, elem);
	}
	return 0;
}

#else
typedef int prevent_empty_translation_unit;
#endif
