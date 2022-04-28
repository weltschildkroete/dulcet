/*
 * Copyright (c) 2022 Leonardo Duarte
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef _DULCET_H
#define _DULCET_H

struct dulcet_term;

struct dulcet_var {
	unsigned int index;
};

struct dulcet_abs {
	struct dulcet_term *m;
};

struct dulcet_app {
	struct dulcet_term *m;
	struct dulcet_term *n;
};

enum dulcet_term_kind {
	DULCET_TERM_KIND_VAR,
	DULCET_TERM_KIND_ABS,
	DULCET_TERM_KIND_APP,
};

struct dulcet_term {
	enum dulcet_term_kind kind;

	union {
		struct dulcet_var var;
		struct dulcet_abs abs;
		struct dulcet_app app;
	};
};

struct dulcet_term *dulcet_alloc_var(unsigned int index);
struct dulcet_term *dulcet_alloc_abs(struct dulcet_term *m);
struct dulcet_term *dulcet_alloc_app(struct dulcet_term *m, struct dulcet_term *n);

struct dulcet_term *dulcet_term_copy(const struct dulcet_term *t);
void dulcet_term_free(struct dulcet_term *t);

int dulcet_term_eq(struct dulcet_term *a, struct dulcet_term *b);

void dulcet_term_print_classic(const struct dulcet_term *t);
void dulcet_term_print_de_bruijn(const struct dulcet_term *t);
void dulcet_term_sprint_classic(char *buf, unsigned int max_size, const struct dulcet_term *t);

void dulcet_apply(struct dulcet_term *t, struct dulcet_term *rhs);
void dulcet_eval(struct dulcet_term *t);

void dulcet_beta_cbn(struct dulcet_term *t);
void dulcet_beta_nor(struct dulcet_term *t);
void dulcet_beta_app(struct dulcet_term *t);

#endif // _DULCET_H
