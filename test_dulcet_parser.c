/*
 * Copyright (c) 2022 Leonardo Duarte
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "dulcet.h"
#include "dulcet_parser.h"

#define ZIDANE_IMPLEMENTATION
#include "zidane.h"

#define VAR(x) dulcet_alloc_var(x)
#define ABS(m) dulcet_alloc_abs(m)
#define APP(m, n) dulcet_alloc_app(m, n)

ZIDANE_TEST(parse_de_bruijn_var)
{
	const char input[] = "1";
	struct dulcet_term *expected = VAR(1);
	struct dulcet_term *actual = dulcet_parse_de_bruijn(input, sizeof(input) / sizeof(*input));

	ZIDANE_VERIFY(dulcet_term_eq(actual, expected));

	dulcet_term_free(actual);
	dulcet_term_free(expected);
}

ZIDANE_TEST(parse_de_bruijn_abs)
{
	const char input[] = "\\\\1";
	struct dulcet_term *expected = ABS(ABS(VAR(1)));
	struct dulcet_term *actual = dulcet_parse_de_bruijn(input, sizeof(input) / sizeof(*input));

	ZIDANE_VERIFY(dulcet_term_eq(actual, expected));

	dulcet_term_free(actual);
	dulcet_term_free(expected);
}

ZIDANE_TEST(parse_de_bruijn_app)
{
	const char input[] = "(\\\\1) (\\1)";
	struct dulcet_term *expected = APP(ABS(ABS(VAR(1))), ABS(VAR(1)));
	struct dulcet_term *actual = dulcet_parse_de_bruijn(input, sizeof(input) / sizeof(*input));

	ZIDANE_VERIFY(dulcet_term_eq(actual, expected));

	dulcet_term_free(actual);
	dulcet_term_free(expected);
}

ZIDANE_TEST(parse_de_bruijn_app_associativity)
{
	const char input[] = "(\\\\1) (\\1) (\\1)";
	struct dulcet_term *expected = APP(APP(ABS(ABS(VAR(1))), ABS(VAR(1))), ABS(VAR(1)));
	struct dulcet_term *actual = dulcet_parse_de_bruijn(input, sizeof(input) / sizeof(*input));

	ZIDANE_VERIFY(dulcet_term_eq(actual, expected));

	dulcet_term_free(actual);
	dulcet_term_free(expected);
}

ZIDANE_TEST(parse_de_bruijn_parenthesized)
{
	const char input[] = "(\\\\1) ((\\1) (\\1))";
	struct dulcet_term *expected = APP(ABS(ABS(VAR(1))), APP(ABS(VAR(1)), ABS(VAR(1))));
	struct dulcet_term *actual = dulcet_parse_de_bruijn(input, sizeof(input) / sizeof(*input));

	ZIDANE_VERIFY(dulcet_term_eq(actual, expected));

	dulcet_term_free(actual);
	dulcet_term_free(expected);
}