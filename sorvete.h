/*
 * Copyright (c) 2022 Leonardo Duarte
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef _SORVETE_H
#define _SORVETE_H

#include <stdbool.h>

struct sorvete_sv {
	unsigned int size;
	const char *data;
};

#define SORVETE_SV_NULL sv_from_parts(NULL, 0)
#define SORVETE_SV(cstr_lit) sorvete_sv_from_parts((cstr_lit), sizeof(cstr_lit) - 1)
#define SORVETE_SV_STATIC(cstr_lit)              \
	{                                        \
		sizeof(cstr_lit) - 1, (cstr_lit) \
	}

#define SORVETE_SV_FMT "%.*s"
#define SORVETE_SV_ARG(sv) (int) (sv).size, (sv).data

#ifdef __cplusplus
extern "C" {
#endif

struct sorvete_sv sorvete_sv_from_parts(const char *data, unsigned int size);
struct sorvete_sv sorvete_sv_from_cstr(const char *cstr);
bool sorvete_sv_eq(struct sorvete_sv sv1, struct sorvete_sv sv2);

#ifdef __cplusplus
}
#endif

#endif // _SORVETE_H
