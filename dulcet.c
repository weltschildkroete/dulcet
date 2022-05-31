/*
 * Copyright (c) 2022 Leonardo Duarte
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "dulcet.h"

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

#define __DULCET_TRY(rc, condition)             \
	do {                                    \
		if (((rc) = (condition)) < 0) { \
			return (rc);            \
		}                               \
	} while (0)

static int __dulcet_term_fprint_classic_rec(const struct dulcet_term *t, FILE *fp,
					    unsigned int context_precedence, unsigned int depth)
{
	if (!t) {
		return -1;
	}

	int chars_written = 0;
	int rc;

	switch (t->kind) {
	case DULCET_TERM_KIND_VAR:
		if (depth >= t->var.index) {
			__DULCET_TRY(rc, fprintf(fp, "%c", 'a' + depth - t->var.index));
			chars_written += rc;
		} else {
			__DULCET_TRY(rc, fprintf(fp, "%c", 'a' + t->var.index - 1));
			chars_written += rc;
		}
		break;
	case DULCET_TERM_KIND_ABS:
		if (context_precedence > 1) {
			__DULCET_TRY(rc, fprintf(fp, "("));
			chars_written += rc;
		}

		__DULCET_TRY(rc, fprintf(fp, "%s%c.", __DULCET_LAMBDA, 'a' + depth));
		chars_written += rc;

		__DULCET_TRY(rc, __dulcet_term_fprint_classic_rec(t->abs.m, fp, 0, depth + 1));
		chars_written += rc;

		if (context_precedence > 1) {
			__DULCET_TRY(rc, fprintf(fp, ")"));
			chars_written += rc;
		}
		break;
	case DULCET_TERM_KIND_APP:
		if (context_precedence == 3) {
			__DULCET_TRY(rc, fprintf(fp, "("));
			chars_written += rc;
		}

		__DULCET_TRY(rc, __dulcet_term_fprint_classic_rec(t->app.m, fp, 2, depth));
		chars_written += rc;

		__DULCET_TRY(rc, fprintf(fp, " "));
		chars_written += rc;

		__DULCET_TRY(rc, __dulcet_term_fprint_classic_rec(t->app.n, fp, 3, depth));
		chars_written += rc;

		if (context_precedence == 3) {
			__DULCET_TRY(rc, fprintf(fp, ")"));
			chars_written += rc;
		}
		break;
	default:
		return -1;
	}

	return chars_written;
}

static int __dulcet_term_sprint_classic_rec(const struct dulcet_term *t, char *buf,
					    unsigned int context_precedence, unsigned int depth)
{
	if (!t) {
		return -1;
	}

	int chars_written = 0;
	int rc;

	switch (t->kind) {
	case DULCET_TERM_KIND_VAR:
		if (depth >= t->var.index) {
			__DULCET_TRY(
				rc, sprintf(buf + chars_written, "%c", 'a' + depth - t->var.index));
			chars_written += rc;
		} else {
			__DULCET_TRY(rc,
				     sprintf(buf + chars_written, "%c", 'a' + t->var.index - 1));
			chars_written += rc;
		}
		break;
	case DULCET_TERM_KIND_ABS:
		if (context_precedence > 1) {
			__DULCET_TRY(rc, sprintf(buf + chars_written, "("));
			chars_written += rc;
		}

		__DULCET_TRY(rc,
			     sprintf(buf + chars_written, "%s%c.", __DULCET_LAMBDA, 'a' + depth));
		chars_written += rc;

		__DULCET_TRY(rc, __dulcet_term_sprint_classic_rec(t->abs.m, buf + chars_written, 0,
								  depth + 1));
		chars_written += rc;

		if (context_precedence > 1) {
			__DULCET_TRY(rc, sprintf(buf + chars_written, ")"));
			chars_written += rc;
		}
		break;
	case DULCET_TERM_KIND_APP:
		if (context_precedence == 3) {
			__DULCET_TRY(rc, sprintf(buf + chars_written, "("));
			chars_written += rc;
		}

		__DULCET_TRY(rc, __dulcet_term_sprint_classic_rec(t->app.m, buf + chars_written, 2,
								  depth));
		chars_written += rc;

		__DULCET_TRY(rc, sprintf(buf + chars_written, " "));
		chars_written += rc;

		__DULCET_TRY(rc, __dulcet_term_sprint_classic_rec(t->app.n, buf + chars_written, 3,
								  depth));
		chars_written += rc;

		if (context_precedence == 3) {
			__DULCET_TRY(rc, sprintf(buf + chars_written, ")"));
			chars_written += rc;
		}
		break;
	default:
		return -1;
	}

	return chars_written;
}

static int __dulcet_term_fprint_de_bruijn_rec(const struct dulcet_term *t, FILE *fp,
					      unsigned int context_precedence, unsigned int depth)
{
	if (!t) {
		return -1;
	}

	int chars_written = 0;
	int rc;

	switch (t->kind) {
	case DULCET_TERM_KIND_VAR:
		__DULCET_TRY(rc, fprintf(fp, "%d", t->var.index));
		chars_written += rc;
		break;
	case DULCET_TERM_KIND_ABS:
		if (context_precedence > 1) {
			__DULCET_TRY(rc, fprintf(fp, "("));
			chars_written += rc;
		}

		__DULCET_TRY(rc, fprintf(fp, "%s", __DULCET_LAMBDA));
		chars_written += rc;

		__DULCET_TRY(rc, __dulcet_term_fprint_de_bruijn_rec(t->abs.m, fp, 0, depth));
		chars_written += rc;

		if (context_precedence > 1) {
			__DULCET_TRY(rc, fprintf(fp, ")"));
			chars_written += rc;
		}
		break;
	case DULCET_TERM_KIND_APP:
		if (context_precedence == 3) {
			__DULCET_TRY(rc, fprintf(fp, "("));
			chars_written += rc;
		}

		__DULCET_TRY(rc, __dulcet_term_fprint_de_bruijn_rec(t->app.m, fp, 2, depth));
		chars_written += rc;

		__DULCET_TRY(rc, fprintf(fp, " "));
		chars_written += rc;

		__DULCET_TRY(rc, __dulcet_term_fprint_de_bruijn_rec(t->app.n, fp, 3, depth));
		chars_written += rc;

		if (context_precedence == 3) {
			__DULCET_TRY(rc, fprintf(fp, ")"));
			chars_written += rc;
		}
		break;
	default:
		return -1;
	}

	return chars_written;
}

static int __dulcet_term_sprint_de_bruijn_rec(const struct dulcet_term *t, char *buf,
					      unsigned int context_precedence, unsigned int depth)
{
	if (!t) {
		return -1;
	}

	int chars_written = 0;
	int rc;

	switch (t->kind) {
	case DULCET_TERM_KIND_VAR:
		__DULCET_TRY(rc, sprintf(buf + chars_written, "%d", t->var.index));
		chars_written += rc;
		break;
	case DULCET_TERM_KIND_ABS:
		if (context_precedence > 1) {
			__DULCET_TRY(rc, sprintf(buf + chars_written, "("));
			chars_written += rc;
		}

		__DULCET_TRY(rc, sprintf(buf + chars_written, "%s", __DULCET_LAMBDA));
		chars_written += rc;

		__DULCET_TRY(rc, __dulcet_term_sprint_de_bruijn_rec(t->abs.m, buf + chars_written,
								    0, depth));
		chars_written += rc;

		if (context_precedence > 1) {
			__DULCET_TRY(rc, sprintf(buf + chars_written, ")"));
			chars_written += rc;
		}
		break;
	case DULCET_TERM_KIND_APP:
		if (context_precedence == 3) {
			__DULCET_TRY(rc, sprintf(buf + chars_written, "("));
			chars_written += rc;
		}

		__DULCET_TRY(rc, __dulcet_term_sprint_de_bruijn_rec(t->app.m, buf + chars_written,
								    2, depth));
		chars_written += rc;

		__DULCET_TRY(rc, sprintf(buf + chars_written, " "));
		chars_written += rc;

		__DULCET_TRY(rc, __dulcet_term_sprint_de_bruijn_rec(t->app.n, buf + chars_written,
								    3, depth));
		chars_written += rc;

		if (context_precedence == 3) {
			__DULCET_TRY(rc, sprintf(buf + chars_written, ")"));
			chars_written += rc;
		}
		break;
	default:
		return -1;
	}

	return chars_written;
}

int dulcet_term_print_classic(const struct dulcet_term *t)
{
	return __dulcet_term_fprint_classic_rec(t, stdout, 0, 0);
}

int dulcet_term_print_de_bruijn(const struct dulcet_term *t)
{
	return __dulcet_term_fprint_de_bruijn_rec(t, stdout, 0, 0);
}

int dulcet_term_fprint_classic(const struct dulcet_term *t, FILE *fp)
{
	return __dulcet_term_fprint_classic_rec(t, fp, 0, 0);
}

int dulcet_term_fprint_de_bruijn(const struct dulcet_term *t, FILE *fp)
{
	return __dulcet_term_fprint_de_bruijn_rec(t, fp, 0, 0);
}

int dulcet_term_sprint_classic(const struct dulcet_term *t, char *buf)
{
	int rc;
	__DULCET_TRY(rc, __dulcet_term_sprint_classic_rec(t, buf, 0, 0));

	buf[rc] = '\0';

	return rc;
}

int dulcet_term_sprint_de_bruijn(const struct dulcet_term *t, char *buf)
{
	int rc;
	__DULCET_TRY(rc, __dulcet_term_sprint_de_bruijn_rec(t, buf, 0, 0));

	buf[rc] = '\0';

	return rc;
}

#undef __DULCET_TRY

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
