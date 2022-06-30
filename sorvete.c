/*
 * Copyright (c) 2022 Leonardo Duarte
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <string.h>

#include "sorvete.h"

struct sorvete_sv sorvete_sv_from_parts(const char *data, unsigned int size)
{
	return (struct sorvete_sv) { size, data };
}

struct sorvete_sv sorvete_sv_from_cstr(const char *cstr)
{
	return sorvete_sv_from_parts(cstr, strlen(cstr));
}

bool sorvete_sv_eq(struct sorvete_sv sv1, struct sorvete_sv sv2)
{
	if (sv1.size != sv2.size) {
		return false;
	}
	return memcmp(sv1.data, sv2.data, sv1.size) == 0;
}
