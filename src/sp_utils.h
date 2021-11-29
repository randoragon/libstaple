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
 *
 * -------------------------------------
 * This header defines functions that should be exposed to the library user, but
 * do not belong in just one module or data structure.
 */
#ifndef STAPLE_UTILS_H
#define STAPLE_UTILS_H

int sp_free(void *addr);

#endif /* STAPLE_UTILS_H */
