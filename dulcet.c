/*
 * Copyright (c) 2022 Leonardo Duarte
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "dulcet.h"

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

struct dulcet_term *dulcet_alloc_var(unsigned int index)
{
	struct dulcet_term *t = malloc(sizeof(*t));
	t->kind = DULCET_TERM_KIND_VAR;
	t->var = (struct dulcet_var) { index };

	return t;
}

struct dulcet_term *dulcet_alloc_abs(struct dulcet_term *m)
{
	struct dulcet_term *t = malloc(sizeof(*t));
	t->kind = DULCET_TERM_KIND_ABS;
	t->abs = (struct dulcet_abs) { m };

	return t;
}

struct dulcet_term *dulcet_alloc_app(struct dulcet_term *m, struct dulcet_term *n)
{
	struct dulcet_term *t = malloc(sizeof(*t));
	t->kind = DULCET_TERM_KIND_APP;
	t->app = (struct dulcet_app) { m, n };

	return t;
}

struct dulcet_term *dulcet_term_copy(const struct dulcet_term *t)
{
	struct dulcet_term *s;

	assert(t);

	switch (t->kind) {
	case DULCET_TERM_KIND_VAR:
		s = dulcet_alloc_var(t->var.index);
		break;
	case DULCET_TERM_KIND_ABS:
		s = dulcet_alloc_abs(dulcet_term_copy(t->abs.m));
		break;
	case DULCET_TERM_KIND_APP:
		s = dulcet_alloc_app(dulcet_term_copy(t->app.m), dulcet_term_copy(t->app.n));
		break;
	default:
		fprintf(stderr, "dulcet: fatal error\n");
		exit(1);
	}

	return s;
}

void dulcet_term_free(struct dulcet_term *t)
{
	assert(t);

	switch (t->kind) {
	case DULCET_TERM_KIND_VAR:
		break;
	case DULCET_TERM_KIND_ABS:
		dulcet_term_free(t->abs.m);
		break;
	case DULCET_TERM_KIND_APP:
		dulcet_term_free(t->app.m);
		dulcet_term_free(t->app.n);
		break;
	default:
		fprintf(stderr, "dulcet: fatal error\n");
		exit(1);
	}

	free(t);
}

int dulcet_term_eq(struct dulcet_term *a, struct dulcet_term *b)
{
	assert(a);
	assert(b);

	if (a->kind != b->kind) {
		return 0;
	}

	switch (a->kind) {
	case DULCET_TERM_KIND_VAR:
		if (a->var.index != b->var.index) {
			return 0;
		}
		break;
	case DULCET_TERM_KIND_ABS:
		if (!dulcet_term_eq(a->abs.m, b->abs.m)) {
			return 0;
		}
		break;
	case DULCET_TERM_KIND_APP:
		if (!dulcet_term_eq(a->app.m, b->app.m) || !dulcet_term_eq(a->app.n, b->app.n)) {
			return 0;
		}
		break;
	default:
		fprintf(stderr, "dulcet: fatal error\n");
		exit(1);
	}

	return 1;
}

#if 1
static const char *__DULCET_LAMBDA = "λ";
#else
static const char *__DULCET_LAMBDA = "\\";
#endif

static void __dulcet_term_print_pretty_rec(const struct dulcet_term *t,
					   unsigned int context_precedence, unsigned int depth)
{
	assert(t);

	switch (t->kind) {
	case DULCET_TERM_KIND_VAR:
		if (depth >= t->var.index) {
			printf("%c", 'a' + depth - t->var.index);
		} else {
			printf("%c", 'a' + t->var.index - 1);
		}
		break;
	case DULCET_TERM_KIND_ABS:
		if (context_precedence > 1) {
			printf("(");
		}
		printf("%s%c.", __DULCET_LAMBDA, 'a' + depth);
		__dulcet_term_print_pretty_rec(t->abs.m, 0, depth + 1);
		if (context_precedence > 1) {
			printf(")");
		}
		break;
	case DULCET_TERM_KIND_APP:
		if (context_precedence == 3) {
			printf("(");
		}
		__dulcet_term_print_pretty_rec(t->app.m, 2, depth);
		printf(" ");
		__dulcet_term_print_pretty_rec(t->app.n, 3, depth);
		if (context_precedence == 3) {
			printf(")");
		}
		break;
	default:
		fprintf(stderr, "dulcet: fatal error\n");
		exit(1);
	}
}

void dulcet_term_print_classic(const struct dulcet_term *t)
{
	__dulcet_term_print_pretty_rec(t, 0, 0);
	printf("\n");
}

static void __dulcet_term_print_de_brujin_rec(const struct dulcet_term *t,
					      unsigned int context_precedence, unsigned int depth)
{
	assert(t);

	switch (t->kind) {
	case DULCET_TERM_KIND_VAR:
		printf("%d", t->var.index);
		break;
	case DULCET_TERM_KIND_ABS:
		if (context_precedence > 1) {
			printf("(");
		}
		printf("%s", __DULCET_LAMBDA);
		__dulcet_term_print_de_brujin_rec(t->abs.m, 0, depth);
		if (context_precedence > 1) {
			printf(")");
		}
		break;
	case DULCET_TERM_KIND_APP:
		if (context_precedence == 3) {
			printf("(");
		}
		__dulcet_term_print_de_brujin_rec(t->app.m, 2, depth);
		printf(" ");
		__dulcet_term_print_de_brujin_rec(t->app.n, 3, depth);
		if (context_precedence == 3) {
			printf(")");
		}
		break;
	default:
		fprintf(stderr, "dulcet: fatal error\n");
		exit(1);
	}
}

void dulcet_term_print_de_brujin(const struct dulcet_term *t)
{
	__dulcet_term_print_de_brujin_rec(t, 0, 0);
	printf("\n");
}

// FIXME: return number of characters written
static void __dulcet_term_sprint_classic_rec(char *buf, unsigned int max_size,
					     const struct dulcet_term *t,
					     unsigned int context_precedence, unsigned int depth,
					     unsigned int *pos)
{
	assert(t);

	if (*pos >= max_size) {
		// FIXME: don't write more than max_size
	}

	switch (t->kind) {
	case DULCET_TERM_KIND_VAR:
		if (depth >= t->var.index) {
			buf[*pos] = 'a' + depth - t->var.index;
		} else {
			buf[*pos] = 'a' + t->var.index - 1;
		}
		*pos += 1;
		break;
	case DULCET_TERM_KIND_ABS:
		if (context_precedence > 1) {
			buf[*pos] = '(';
			*pos += 1;
		}
		int n = 0;
		if ((n = sprintf(buf + *pos, "%s", __DULCET_LAMBDA)) <= 0) {
			fprintf(stderr, "dulcet: fatal error\n");
		}
		*pos += n;
		buf[*pos] = 'a' + depth;
		*pos += 1;
		buf[*pos] = '.';
		*pos += 1;
		__dulcet_term_sprint_classic_rec(buf, max_size, t->abs.m, 0, depth + 1, pos);
		if (context_precedence > 1) {
			buf[*pos] = ')';
			*pos += 1;
		}
		break;
	case DULCET_TERM_KIND_APP:
		if (context_precedence == 3) {
			buf[*pos] = '(';
			*pos += 1;
		}
		__dulcet_term_sprint_classic_rec(buf, max_size, t->app.m, 2, depth, pos);
		buf[*pos] = ' ';
		*pos += 1;
		__dulcet_term_sprint_classic_rec(buf, max_size, t->app.n, 3, depth, pos);
		if (context_precedence == 3) {
			buf[*pos] = ')';
			*pos += 1;
		}
		break;
	default:
		fprintf(stderr, "dulcet: fatal error\n");
		exit(1);
	}
}

void dulcet_term_sprint_classic(char *buf, unsigned int max_size, const struct dulcet_term *t)
{
	unsigned int pos = 0;

	__dulcet_term_sprint_classic_rec(buf, max_size, t, 0, 0, &pos);

	buf[pos] = '\0';
}

static void __dulcet_update_free_variables(struct dulcet_term *t, unsigned int added_depth,
					   unsigned int own_depth)
{
	assert(t);

	switch (t->kind) {
	case DULCET_TERM_KIND_VAR:
		if (t->var.index > own_depth) {
			t->var.index += added_depth;
		}
		break;
	case DULCET_TERM_KIND_ABS:
		__dulcet_update_free_variables(t->abs.m, added_depth, own_depth + 1);
		break;
	case DULCET_TERM_KIND_APP:
		__dulcet_update_free_variables(t->app.m, added_depth, own_depth);
		__dulcet_update_free_variables(t->app.n, added_depth, own_depth);
		break;
	default:
		fprintf(stderr, "dulcet: fatal error\n");
		exit(1);
	}
}

static void __dulcet_apply_rec(struct dulcet_term *t, const struct dulcet_term *rhs,
			       unsigned int depth)
{
	switch (t->kind) {
	case DULCET_TERM_KIND_VAR:
		if (t->var.index == depth) {
			struct dulcet_term *tmp = dulcet_term_copy(rhs);
			*t = *tmp;
			free(tmp);
			__dulcet_update_free_variables(t, depth - 1, 0);
		} else if (t->var.index > depth) {
			t->var.index -= 1;
		}
		break;
	case DULCET_TERM_KIND_ABS:
		__dulcet_apply_rec(t->abs.m, rhs, depth + 1);
		break;
	case DULCET_TERM_KIND_APP:
		__dulcet_apply_rec(t->app.m, rhs, depth);
		__dulcet_apply_rec(t->app.n, rhs, depth);
		break;
	default:
		fprintf(stderr, "dulcet: fatal error\n");
		exit(1);
	}
}

void dulcet_apply(struct dulcet_term *t, struct dulcet_term *rhs)
{
	assert(t && t->kind == DULCET_TERM_KIND_ABS);

	struct dulcet_term *tmp = t->abs.m;

	__dulcet_apply_rec(t, rhs, 0);

	dulcet_term_free(rhs);

	*t = *t->abs.m;

	free(tmp);
}

void dulcet_eval(struct dulcet_term *t)
{
	assert(t && t->kind == DULCET_TERM_KIND_APP);

	struct dulcet_term *tmp = t->app.m;

	dulcet_apply(t->app.m, t->app.n);

	*t = *t->app.m;

	free(tmp);
}

void dulcet_beta_cbn(struct dulcet_term *t)
{
	assert(t);

	if (t->kind == DULCET_TERM_KIND_APP) {
		dulcet_beta_cbn(t->app.m);

		if (t->app.m && t->app.m->kind == DULCET_TERM_KIND_ABS) {
			dulcet_eval(t);

			dulcet_beta_cbn(t);
		}
	}
}

void dulcet_beta_nor(struct dulcet_term *t)
{
	assert(t);

	switch (t->kind) {
	case DULCET_TERM_KIND_VAR:
		break;
	case DULCET_TERM_KIND_ABS:
		dulcet_beta_nor(t->abs.m);
		break;
	case DULCET_TERM_KIND_APP:
		dulcet_beta_cbn(t->app.m);

		if (t->app.m && t->app.m->kind == DULCET_TERM_KIND_ABS) {
			dulcet_eval(t);

			dulcet_beta_nor(t);
		} else {
			dulcet_beta_nor(t->app.m);
			dulcet_beta_nor(t->app.n);
		}
		break;
	default:
		fprintf(stderr, "dulcet: fatal error\n");
		exit(1);
	}
}

void dulcet_beta_app(struct dulcet_term *t)
{
	assert(t);

	switch (t->kind) {
	case DULCET_TERM_KIND_VAR:
		break;
	case DULCET_TERM_KIND_ABS:
		dulcet_beta_app(t->abs.m);
		break;
	case DULCET_TERM_KIND_APP:
		dulcet_beta_app(t->app.m);
		dulcet_beta_app(t->app.n);

		if (t->app.m && t->app.m->kind == DULCET_TERM_KIND_ABS) {
			dulcet_eval(t);

			dulcet_beta_app(t);
		}
		break;
	default:
		fprintf(stderr, "dulcet: fatal error\n");
		exit(1);
	}
}
