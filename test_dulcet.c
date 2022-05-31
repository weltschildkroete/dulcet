/*
 * Copyright (c) 2022 Leonardo Duarte
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <string.h>

#define ZIDANE_IMPLEMENTATION
#include "zidane.h"

#include "dulcet.h"

#define VAR(x) dulcet_alloc_var(x)
#define ABS(m) dulcet_alloc_abs(m)
#define APP(m, n) dulcet_alloc_app(m, n)

ZIDANE_TEST(sanity)
{
	struct dulcet_term *x = APP(ABS(VAR(1)), VAR(2));

	dulcet_term_free(x);
}

ZIDANE_TEST(term_eq)
{
	struct dulcet_term *x = APP(ABS(VAR(1)), VAR(2));
	struct dulcet_term *y = APP(ABS(VAR(1)), VAR(2));

	ZIDANE_VERIFY(dulcet_term_eq(x, y));

	dulcet_term_free(y);
	dulcet_term_free(x);
}

ZIDANE_TEST(beta_nor_pred_succ)
{
	struct dulcet_term *succ = ABS(ABS(ABS(APP(VAR(2), APP(APP(VAR(3), VAR(2)), VAR(1))))));
	struct dulcet_term *pred = ABS(ABS(
		ABS(APP(APP(APP(VAR(3), ABS(ABS(APP(VAR(1), APP(VAR(2), VAR(4)))))), ABS(VAR(2))),
			ABS(VAR(1))))));

	struct dulcet_term *actual = APP(pred, succ);
	dulcet_beta_nor(actual);

	struct dulcet_term *expected = ABS(ABS(VAR(1)));

	ZIDANE_VERIFY(dulcet_term_eq(actual, expected));

	dulcet_term_free(expected);
	dulcet_term_free(actual);
}

ZIDANE_TEST(term_sprint_classic)
{
	struct dulcet_term *succ = ABS(ABS(ABS(APP(VAR(2), APP(APP(VAR(3), VAR(2)), VAR(1))))));

	char buf[BUFSIZ];
	int rc = dulcet_term_sprint_classic(succ, buf);

	ZIDANE_VERIFY(rc > 0);
	ZIDANE_VERIFY((unsigned long) rc == strlen(buf));
	ZIDANE_VERIFY(strcmp(buf, "λa.λb.λc.b (a b c)") == 0);

	dulcet_term_free(succ);
}

ZIDANE_TEST(term_sprint_de_bruijn)
{
	struct dulcet_term *succ = ABS(ABS(ABS(APP(VAR(2), APP(APP(VAR(3), VAR(2)), VAR(1))))));

	char buf[BUFSIZ];
	int rc = dulcet_term_sprint_de_bruijn(succ, buf);

	ZIDANE_VERIFY(rc > 0);
	ZIDANE_VERIFY((unsigned long) rc == strlen(buf));
	ZIDANE_VERIFY(strcmp(buf, "λλλ2 (3 2 1)") == 0);

	dulcet_term_free(succ);
}

ZIDANE_TEST(term_sprint_fail)
{
	char buf[BUFSIZ];
	int rc = dulcet_term_sprint_classic(NULL, buf);

	ZIDANE_VERIFY(rc < 0);
}
