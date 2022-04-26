/*
 * Copyright (c) 2022 Leonardo Duarte
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#define ZIDANE_IMPLEMENTATION
#include "zidane.h"

#include "dulcet.h"

ZIDANE_TEST(sanity)
{
	struct dulcet_term *x =
		dulcet_alloc_app(dulcet_alloc_abs(dulcet_alloc_var(1)), dulcet_alloc_var(2));

	dulcet_term_free(x);
}

ZIDANE_TEST(term_eq)
{
	struct dulcet_term *x =
		dulcet_alloc_app(dulcet_alloc_abs(dulcet_alloc_var(1)), dulcet_alloc_var(2));
	struct dulcet_term *y =
		dulcet_alloc_app(dulcet_alloc_abs(dulcet_alloc_var(1)), dulcet_alloc_var(2));

	ZIDANE_VERIFY(dulcet_term_eq(x, y));

	dulcet_term_free(y);
	dulcet_term_free(x);
}

ZIDANE_TEST(beta_nor_pred_succ)
{
	struct dulcet_term *succ =
		dulcet_alloc_abs(dulcet_alloc_abs(dulcet_alloc_abs(dulcet_alloc_app(
			dulcet_alloc_var(2),
			dulcet_alloc_app(dulcet_alloc_app(dulcet_alloc_var(3), dulcet_alloc_var(2)),
					 dulcet_alloc_var(1))))));
	struct dulcet_term *pred =
		dulcet_alloc_abs(dulcet_alloc_abs(dulcet_alloc_abs(dulcet_alloc_app(
			dulcet_alloc_app(
				dulcet_alloc_app(dulcet_alloc_var(3),
						 dulcet_alloc_abs(dulcet_alloc_abs(dulcet_alloc_app(
							 dulcet_alloc_var(1),
							 dulcet_alloc_app(dulcet_alloc_var(2),
									  dulcet_alloc_var(4)))))),
				dulcet_alloc_abs(dulcet_alloc_var(2))),
			dulcet_alloc_abs(dulcet_alloc_var(1))))));

	struct dulcet_term *actual = dulcet_alloc_app(pred, succ);
	dulcet_beta_nor(actual);

	struct dulcet_term *expected = dulcet_alloc_abs(dulcet_alloc_abs(dulcet_alloc_var(1)));

	ZIDANE_VERIFY(dulcet_term_eq(actual, expected));

	dulcet_term_free(expected);
	dulcet_term_free(actual);
}
