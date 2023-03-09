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
#include "../sp_utils.h"
#include "../internal.h"
#include <stdlib.h>

int sp_free(void *addr)
{
#ifdef STAPLE_DEBUG
	if (addr == NULL) {
		error(("addr is NULL"));
		return 1;
	}
	if (*(void**)addr == NULL) {
		error(("*(void**)addr is NULL"));
		return 2;
	}
#endif
	free(*(void**)addr);
	return 0;
}
