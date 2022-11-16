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
#include "test_struct.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

const char *random_names[] = {
	"Christopher", "Jimmy", "Kyle", "Lori", "Paul",
	"Charles", "Jennie", "Thomas", "Camille", "William",
	"John", "Kory", "Keith", "Henry", "Jodie",
	"Stephen", "Richard", "Allen", "Laura", "Eve",
};

const char *random_surnames[] = {
	"Bailey", "Ortiz", "Turnipseed", "Bulloch", "Richardson",
	"Pham", "Balentine", "Corbett", "Bergeron", "Green",
	"Mangrum", "Roberts", "Berry", "Conn", "Fleetwood",
	"Clayton", "Crutcher", "Newsome", "Reid", "Pless",
};

int data_init(struct data *d)
{
	size_t name_idx, surname_idx;
	name_idx     = IRANGE(0, LEN(random_names)    - 1);
	surname_idx  = IRANGE(0, LEN(random_surnames) - 1);
	d->name      = malloc(32 * sizeof(*d->name));
	if (d->name == NULL) {
		free(d);
		return 1;
	}
	d->surname = malloc(32 * sizeof(*d->name));
	if (d->surname == NULL) {
		free(d->name);
		free(d);
		return 1;
	}
	strcpy(d->name, random_names[name_idx]);
	strcpy(d->surname, random_surnames[surname_idx]);
	d->age = IRANGE(0, 120);
	d->id  = 0;
	return 0;
}

int data_cpy(void *dest, const void *src)
{
	struct data       *p1 = dest;
	const struct data *p2 = src;
	p1->name = malloc(32 * sizeof(*p1->name));
	if (p1->name == NULL) {
		return 1;
	}
	p1->surname = malloc(32 * sizeof(*p1->name));
	if (p1->surname == NULL) {
		free(p1->name);
		return 1;
	}
	strcpy(p1->name,    p2->name);
	strcpy(p1->surname, p2->surname);
	p1->age = p2->age;
	p1->id = p2->id;
	return 0;
}

int data_cpy_bad(void *dest, const void *src)
{
	return 1;
}

int data_dtor(void *d)
{
	struct data *const p = d;
	free(p->name);
	free(p->surname);
	return 0;
}

int data_dtor_bad(void *d)
{
	return 1;
}

int data_cmp(const void *a, const void *b)
{
	const struct data *a_ = (struct data*)a;
	const struct data *b_ = (struct data*)b;
	if (a_->id != b_->id)                 return 1;
	if (a_->age != b_->age)               return 2;
	if (strcmp(a_->name, b_->name))       return 3;
	if (strcmp(a_->surname, b_->surname)) return 4;
	return 0;
}

int data_mutate(void *d, size_t idx)
{
	struct data *p = d;
	p->id = idx % 16 + p->age + p->name[0] * p->surname[0];
	return 0;
}

int data_mutate_bad(void *d, size_t idx)
{
	return 1;
}

int data_verify(void *d, size_t idx)
{
	const struct data *p = d;
	return !(p->id == idx % 16 + p->age + p->name[0] * p->surname[0]);
}

int data_print(const void *d)
{
	const struct data *p = d;
	printf("id %d, %s %s, age %d\n", p->id, p->name, p->surname, p->age);
	return 0;
}

int data_print_bad(const void *d)
{
	return 1;
}
