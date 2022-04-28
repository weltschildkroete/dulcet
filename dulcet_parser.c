/*
 * Copyright (c) 2022 Leonardo Duarte
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>
#include <stdlib.h>

#include "dulcet_parser.h"

#include "dulcet.h"

static struct dulcet_term *__dulcet_parse_parenthesized_de_bruijn(const char **input);

static struct dulcet_term *__dulcet_parse_one_de_bruijn(const char **input)
{
	struct dulcet_term *m = NULL;

	int x = atoi(*input);
	if (x > 0) {
                // FIXME: add the size of the number
		*input += 1;
		m = dulcet_alloc_var(x);
	} else if (**input == '\\') {
		*input += 1;
		struct dulcet_term *inner = __dulcet_parse_parenthesized_de_bruijn(input);
		m = dulcet_alloc_abs(inner);
	}

	return m;
}

static struct dulcet_term *__dulcet_parse_parenthesized_de_bruijn(const char **input)
{
	struct dulcet_term *m = NULL;

	while (**input != '\0' && **input != ')') {
		struct dulcet_term *n = NULL;

		while (**input == ' ' || **input == '\n') {
			*input += 1;
		}

		if (**input == '(') {
			*input += 1;
			n = __dulcet_parse_parenthesized_de_bruijn(input);

			if (**input != ')') {
				fprintf(stderr, "dulcet_parse_de_bruijn: fatal error\n");
				fprintf(stderr, "Missing `)`\n");
				exit(1);
			}
			*input += 1;
		} else {
			n = __dulcet_parse_one_de_bruijn(input);
		}

		if (m == NULL) {
			m = n;
		} else if (n != NULL) {
			m = dulcet_alloc_app(m, n);
		}
	}

	return m;
}

struct dulcet_term *dulcet_parse_de_bruijn(const char *input, unsigned int input_len)
{
	(void) input_len;

	return __dulcet_parse_parenthesized_de_bruijn(&input);
}
