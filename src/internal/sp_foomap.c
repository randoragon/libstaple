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
#include "../internal.h"
#include <stdlib.h>

int sp_foomap(void *buf, size_t size, size_t elem_size, int (*foo)(void*))
{
	const void *const end = (char*)buf + size * elem_size;
	char *p = buf;
	while (p != end) {
		int err;
		if ((err = foo(p))) {
			error(("callback function foo returned %d (non-0)", err));
			return 1;
		}
		p += elem_size;
	}
	return 0;
}
