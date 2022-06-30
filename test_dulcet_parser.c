/*
 * Copyright (c) 2022 Leonardo Duarte
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "dulcet.h"
#include "dulcet_parser.h"

#define ZIDANE_IMPLEMENTATION
#include "zidane.h"

#define ARRAY_SIZE(xs) (sizeof(xs) / sizeof(*(xs)))

#define VAR(x) dulcet_alloc_var(x)
#define ABS(m) dulcet_alloc_abs(m)
#define APP(m, n) dulcet_alloc_app(m, n)

ZIDANE_TEST(parse_de_bruijn_var)
{
	const char input[] = "1";
	struct dulcet_term *expected = VAR(1);

	struct dulcet_parse_result result = dulcet_parse_de_bruijn(input, ARRAY_SIZE(input));
	ZIDANE_VERIFY(result.kind == DULCET_PARSE_OK);

	struct dulcet_term *actual = result.value;
	ZIDANE_VERIFY(dulcet_term_eq(actual, expected));

	dulcet_term_free(actual);
	dulcet_term_free(expected);
}

ZIDANE_TEST(parse_de_bruijn_abs)
{
	const char input[] = "\\\\1";
	struct dulcet_term *expected = ABS(ABS(VAR(1)));

	struct dulcet_parse_result result = dulcet_parse_de_bruijn(input, ARRAY_SIZE(input));
	ZIDANE_VERIFY(result.kind == DULCET_PARSE_OK);

	struct dulcet_term *actual = result.value;
	ZIDANE_VERIFY(dulcet_term_eq(actual, expected));

	dulcet_term_free(actual);
	dulcet_term_free(expected);
}

ZIDANE_TEST(parse_de_bruijn_app)
{
	const char input[] = "(\\\\1) (\\1)";
	struct dulcet_term *expected = APP(ABS(ABS(VAR(1))), ABS(VAR(1)));

	struct dulcet_parse_result result = dulcet_parse_de_bruijn(input, ARRAY_SIZE(input));
	ZIDANE_VERIFY(result.kind == DULCET_PARSE_OK);

	struct dulcet_term *actual = result.value;
	ZIDANE_VERIFY(dulcet_term_eq(actual, expected));

	dulcet_term_free(actual);
	dulcet_term_free(expected);
}

ZIDANE_TEST(parse_de_bruijn_app_associativity)
{
	const char input[] = "(\\\\1) (\\1) (\\1)";
	struct dulcet_term *expected = APP(APP(ABS(ABS(VAR(1))), ABS(VAR(1))), ABS(VAR(1)));

	struct dulcet_parse_result result = dulcet_parse_de_bruijn(input, ARRAY_SIZE(input));
	ZIDANE_VERIFY(result.kind == DULCET_PARSE_OK);

	struct dulcet_term *actual = result.value;
	ZIDANE_VERIFY(dulcet_term_eq(actual, expected));

	dulcet_term_free(actual);
	dulcet_term_free(expected);
}

ZIDANE_TEST(parse_de_bruijn_parenthesized)
{
	const char input[] = "(\\\\1) ((\\1) (\\1))";
	struct dulcet_term *expected = APP(ABS(ABS(VAR(1))), APP(ABS(VAR(1)), ABS(VAR(1))));

	struct dulcet_parse_result result = dulcet_parse_de_bruijn(input, ARRAY_SIZE(input));
	ZIDANE_VERIFY(result.kind == DULCET_PARSE_OK);

	struct dulcet_term *actual = result.value;
	ZIDANE_VERIFY(dulcet_term_eq(actual, expected));

	dulcet_term_free(actual);
	dulcet_term_free(expected);
}

ZIDANE_TEST(parse_de_bruijn_unmatched_lparen)
{
	const char input[] = "(";

	struct dulcet_parse_result result = dulcet_parse_de_bruijn(input, ARRAY_SIZE(input));
	ZIDANE_VERIFY(result.kind == DULCET_PARSE_ERROR);
	ZIDANE_VERIFY(result.error.cause == DULCET_PARSE_ERROR_CAUSE_UNMATCHED_PAREN);
	ZIDANE_VERIFY(result.error.line == 1);
	ZIDANE_VERIFY(result.error.column == 1);
}

ZIDANE_TEST(parse_de_bruijn_unexpected_rparen)
{
	const char input[] = ")";

	struct dulcet_parse_result result = dulcet_parse_de_bruijn(input, ARRAY_SIZE(input));
	ZIDANE_VERIFY(result.kind == DULCET_PARSE_ERROR);
	ZIDANE_VERIFY(result.error.cause == DULCET_PARSE_ERROR_CAUSE_UNMATCHED_PAREN);
	ZIDANE_VERIFY(result.error.line == 1);
	ZIDANE_VERIFY(result.error.column == 1);
}

ZIDANE_TEST(parse_de_bruijn_unexpected_rparen_nested)
{
	const char input[] = "((\\1)))";

	struct dulcet_parse_result result = dulcet_parse_de_bruijn(input, ARRAY_SIZE(input));
	ZIDANE_VERIFY(result.kind == DULCET_PARSE_ERROR);
	ZIDANE_VERIFY(result.error.cause == DULCET_PARSE_ERROR_CAUSE_UNMATCHED_PAREN);
	ZIDANE_VERIFY(result.error.line == 1);
	ZIDANE_VERIFY(result.error.column == 7);
}
