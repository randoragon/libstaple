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
#ifndef STAPLE_ERRCODES_H
#define STAPLE_ERRCODES_H

/* This header defines symbolic names for all error codes returned by various
 * functions of the library. That way it's easier for the end-user to capture,
 * identify and handle errors.
 */

/* Invalid argument value */
#define SP_EINVAL 1

/* Insufficient memory */
#define SP_ENOMEM 2

/* Callback function returned an error */
#define SP_ECALLBK 3

/* Index out of range */
#define SP_EINDEX 4

/* Illegal operation */
#define SP_EILLEGAL 5

/* Numerical range exceeded (underflow/overflow) */
#define SP_ERANGE 6

#endif /* STAPLE_ERRCODES_H */