/*
 * Copyright (c) 2022 Leonardo Duarte
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "dulcet_parser.h"

#include "dulcet.h"
#include "sorvete.h"

enum token_kind {
	TOKEN_KIND_IDENT,
	TOKEN_KIND_INT,
	TOKEN_KIND_LPAREN,
	TOKEN_KIND_RPAREN,
	TOKEN_KIND_LAMBDA,
	TOKEN_KIND_DOT,
};

struct location {
	size_t pos;
	size_t line;
	size_t column;
};

struct token {
	enum token_kind kind;
	struct sorvete_sv text;
	struct location loc;
};

static struct token __dulcet_token_begin(enum token_kind kind, const char *text_start,
					 struct location loc)
{
	return (struct token) {
		.kind = kind,
		.text = sorvete_sv_from_parts(text_start, 1),
		.loc = loc,
	};
}

struct tokenization {
	size_t size;
	struct token buf[];
};

static const size_t TOKENIZATION_BUFFER_MAX_SIZE = 1024;

static void __dulcet_tokenization_push_token(struct tokenization *tokenization, struct token tk)
{
	// FIXME: implement the tokenization buffer as a dynamic list
	assert(tokenization->size < TOKENIZATION_BUFFER_MAX_SIZE);

	tokenization->buf[tokenization->size] = tk;
	tokenization->size += 1;
}

enum lexer_state {
	LEXER_STATE_START,
	LEXER_STATE_READ_IDENT,
	LEXER_STATE_READ_INT,
	LEXER_STATE_READ_COMMENT,
};

// clang-format off

#define NUMERIC	  \
	     '0': \
	case '1': \
	case '2': \
	case '3': \
	case '4': \
	case '5': \
	case '6': \
	case '7': \
	case '8': \
	case '9'

#define WHITESPACE \
	     ' ':  \
	case '\n': \
	case '\t'

// clang-format on

static int __dulcet_tokenize(struct tokenization *tokenization, struct sorvete_sv input)
{
	struct location loc = {
		.pos = 0,
		.line = 1,
		.column = 1,
	};

	enum lexer_state state = LEXER_STATE_START;

	struct token tk = { 0 };

	while (loc.pos < input.size) {
		char c = input.data[loc.pos];

		switch (state) {
		case LEXER_STATE_START:
			switch (c) {
			case WHITESPACE:
				break;

			case NUMERIC:
				tk = __dulcet_token_begin(TOKEN_KIND_INT, input.data + loc.pos,
							  loc);
				state = LEXER_STATE_READ_INT;
				break;

			case '(':
				tk = __dulcet_token_begin(TOKEN_KIND_LPAREN, input.data + loc.pos,
							  loc);
				__dulcet_tokenization_push_token(tokenization, tk);
				break;

			case ')':
				tk = __dulcet_token_begin(TOKEN_KIND_RPAREN, input.data + loc.pos,
							  loc);
				__dulcet_tokenization_push_token(tokenization, tk);
				break;

			case '\\':
				tk = __dulcet_token_begin(TOKEN_KIND_LAMBDA, input.data + loc.pos,
							  loc);
				__dulcet_tokenization_push_token(tokenization, tk);
				break;

			case '.':
				tk = __dulcet_token_begin(TOKEN_KIND_DOT, input.data + loc.pos,
							  loc);
				__dulcet_tokenization_push_token(tokenization, tk);
				break;

			case ';':
				state = LEXER_STATE_READ_COMMENT;
				break;

			default:
				tk = __dulcet_token_begin(TOKEN_KIND_IDENT, input.data + loc.pos,
							  loc);
				state = LEXER_STATE_READ_IDENT;
				break;
			}
			break;

		case LEXER_STATE_READ_IDENT:
			switch (c) {
			case WHITESPACE:
			case '(':
			case ')':
			case '\\':
			case '.':
			case ';':
				__dulcet_tokenization_push_token(tokenization, tk);

				state = LEXER_STATE_START;

				loc.pos -= 1;
				loc.column -= 1;

				break;

			default:
				tk.text.size += 1;
				break;
			}
			break;

		case LEXER_STATE_READ_INT:
			switch (c) {
			case NUMERIC:
				tk.text.size += 1;
				break;

			default:
				__dulcet_tokenization_push_token(tokenization, tk);

				state = LEXER_STATE_START;

				loc.pos -= 1;
				loc.column -= 1;

				break;
			}
			break;

		case LEXER_STATE_READ_COMMENT:
			if (c == '\n') {
				state = LEXER_STATE_START;
			}
			break;

		default:
			return -1;
		}

		if (c == '\n') {
			loc.line += 1;
			loc.column = 1;
		} else {
			loc.column += 1;
		}

		loc.pos += 1;
	}

	return 0;
}

struct token_stream {
	size_t pos;
	const struct tokenization *tokenization;
};

#define PARAMETER_STACK_MAX_SIZE 256

struct parsing_context {
	struct token_stream tks;
	unsigned int paren_depth;

	struct sorvete_sv parameter_stack[PARAMETER_STACK_MAX_SIZE];
	unsigned int parameter_stack_size;
};

static struct token __dulcet_current_token(const struct parsing_context *ctx)
{
	return ctx->tks.tokenization->buf[ctx->tks.pos];
}

static struct token __dulcet_next_token(struct parsing_context *ctx)
{
	ctx->tks.pos += 1;
	return __dulcet_current_token(ctx);
}

static void __dulcet_push_parameter(struct parsing_context *ctx, struct sorvete_sv parameter)
{
	ctx->parameter_stack[ctx->parameter_stack_size] = parameter;
	ctx->parameter_stack_size += 1;
}

static void __dulcet_pop_parameter(struct parsing_context *ctx)
{
	ctx->parameter_stack_size -= 1;
}

static long long __dulcet_parameter_to_de_bruijn_index(const struct parsing_context *ctx,
						       struct sorvete_sv parameter)
{
	for (int i = ctx->parameter_stack_size - 1; i >= 0; --i) {
		if (sorvete_sv_eq(ctx->parameter_stack[i], parameter)) {
			return ctx->parameter_stack_size - i;
		}
	}

	return -1;
}

static struct dulcet_parse_result __dulcet_error(enum dulcet_parse_error_cause cause,
						 struct token tk)
{
	return (struct dulcet_parse_result) {
                .kind = DULCET_PARSE_ERROR,
                .error = {
                        .cause = cause,
                        .text_start = tk.text.data,
                        .text_len = tk.text.size,
                        .line = tk.loc.line,
                        .column = tk.loc.column,
                },
        };
}

static struct dulcet_parse_result __dulcet_parse_classic(struct parsing_context *ctx)
{
	struct dulcet_term *m = NULL;

	while (ctx->tks.pos < ctx->tks.tokenization->size) {
		struct token tk = __dulcet_current_token(ctx);

		if (tk.kind == TOKEN_KIND_RPAREN) {
			if (ctx->paren_depth == 0) {
				if (m != NULL) {
					dulcet_term_free(m);
				}

				return __dulcet_error(DULCET_PARSE_ERROR_CAUSE_UNMATCHED_PAREN, tk);
			} else {
				break;
			}
		}

		struct dulcet_term *n = NULL;

		if (tk.kind == TOKEN_KIND_LPAREN) {
			__dulcet_next_token(ctx);

			ctx->paren_depth += 1;

			struct dulcet_parse_result result = __dulcet_parse_classic(ctx);
			if (result.kind == DULCET_PARSE_ERROR) {
				if (m != NULL) {
					dulcet_term_free(m);
				}

				return result;
			}

			ctx->paren_depth -= 1;

			n = result.value;

			if (ctx->tks.pos >= ctx->tks.tokenization->size ||
			    __dulcet_current_token(ctx).kind != TOKEN_KIND_RPAREN) {
				if (m != NULL) {
					dulcet_term_free(m);
				}

				if (n != NULL) {
					dulcet_term_free(n);
				}

				return __dulcet_error(DULCET_PARSE_ERROR_CAUSE_UNMATCHED_PAREN, tk);
			}

			__dulcet_next_token(ctx);
		} else if (tk.kind == TOKEN_KIND_IDENT) {
			__dulcet_next_token(ctx);

			long long index = __dulcet_parameter_to_de_bruijn_index(ctx, tk.text);

			if (index < 0) {
				return __dulcet_error(DULCET_PARSE_ERROR_CAUSE_UNBOUND_VARIABLE,
						      tk);
			}

			n = dulcet_alloc_var(index);
		} else if (tk.kind == TOKEN_KIND_LAMBDA) {
			tk = __dulcet_next_token(ctx);
			if (tk.kind != TOKEN_KIND_IDENT) {
				if (m != NULL) {
					dulcet_term_free(m);
				}

				return __dulcet_error(DULCET_PARSE_ERROR_CAUSE_UNEXPECTED_TOKEN,
						      tk);
			}
			__dulcet_push_parameter(ctx, tk.text);

			tk = __dulcet_next_token(ctx);
			if (tk.kind != TOKEN_KIND_DOT) {
				if (m != NULL) {
					dulcet_term_free(m);
				}

				return __dulcet_error(DULCET_PARSE_ERROR_CAUSE_UNEXPECTED_TOKEN,
						      tk);
			}
			__dulcet_next_token(ctx);

			struct dulcet_parse_result result = __dulcet_parse_classic(ctx);
			if (result.kind == DULCET_PARSE_ERROR) {
				if (m != NULL) {
					dulcet_term_free(m);
				}

				return result;
			}
			__dulcet_pop_parameter(ctx);

			n = dulcet_alloc_abs(result.value);
		}

		if (m == NULL) {
			m = n;
		} else if (n != NULL) {
			m = dulcet_alloc_app(m, n);
		}
	}

	return (struct dulcet_parse_result) {
		.kind = DULCET_PARSE_OK,
		.value = m,
	};
}

static struct dulcet_parse_result __dulcet_parse_de_bruijn(struct parsing_context *ctx)
{
	struct dulcet_term *m = NULL;

	while (ctx->tks.pos < ctx->tks.tokenization->size) {
		struct token tk = __dulcet_current_token(ctx);

		if (tk.kind == TOKEN_KIND_RPAREN) {
			if (ctx->paren_depth == 0) {
				if (m != NULL) {
					dulcet_term_free(m);
				}

				return __dulcet_error(DULCET_PARSE_ERROR_CAUSE_UNMATCHED_PAREN, tk);
			} else {
				break;
			}
		}

		struct dulcet_term *n = NULL;

		if (tk.kind == TOKEN_KIND_LPAREN) {
			__dulcet_next_token(ctx);

			ctx->paren_depth += 1;

			struct dulcet_parse_result result = __dulcet_parse_de_bruijn(ctx);
			if (result.kind == DULCET_PARSE_ERROR) {
				if (m != NULL) {
					dulcet_term_free(m);
				}

				return result;
			}

			ctx->paren_depth -= 1;

			n = result.value;

			if (ctx->tks.pos >= ctx->tks.tokenization->size ||
			    __dulcet_current_token(ctx).kind != TOKEN_KIND_RPAREN) {
				if (m != NULL) {
					dulcet_term_free(m);
				}

				if (n != NULL) {
					dulcet_term_free(n);
				}

				return __dulcet_error(DULCET_PARSE_ERROR_CAUSE_UNMATCHED_PAREN, tk);
			}

			__dulcet_next_token(ctx);
		} else if (tk.kind == TOKEN_KIND_INT) {
			__dulcet_next_token(ctx);

			unsigned int value = 0;
			for (size_t i = 0; i < tk.text.size; ++i) {
				value *= 10;
				value += tk.text.data[i] - '0';
			}

			n = dulcet_alloc_var(value);
		} else if (tk.kind == TOKEN_KIND_LAMBDA) {
			__dulcet_next_token(ctx);

			struct dulcet_parse_result result = __dulcet_parse_de_bruijn(ctx);
			if (result.kind == DULCET_PARSE_ERROR) {
				if (m != NULL) {
					dulcet_term_free(m);
				}

				return result;
			}

			n = dulcet_alloc_abs(result.value);
		}

		if (m == NULL) {
			m = n;
		} else if (n != NULL) {
			m = dulcet_alloc_app(m, n);
		}
	}

	return (struct dulcet_parse_result) {
		.kind = DULCET_PARSE_OK,
		.value = m,
	};
}

struct dulcet_parse_result dulcet_parse_classic(const char *input, unsigned int input_len)
{
	struct tokenization *tokenization = malloc(
		sizeof(struct tokenization) + TOKENIZATION_BUFFER_MAX_SIZE * sizeof(struct token));
	tokenization->size = 0;

	struct sorvete_sv input_sv = sorvete_sv_from_parts(input, input_len);

	int rc = __dulcet_tokenize(tokenization, input_sv);
	if (rc != 0) {
		return (struct dulcet_parse_result) {
			.kind = DULCET_PARSE_ERROR,
			.error = {
                                .cause = DULCET_PARSE_ERROR_CAUSE_UNKNOWN,
                                .line = 1,
                                .column = 1,
                        },
		};
	}

	struct parsing_context ctx = {
                .tks = {
		        .tokenization = tokenization,
		        .pos = 0,
	        },
                .paren_depth = 0,
                .parameter_stack_size = 0,
        };

	struct dulcet_parse_result result = __dulcet_parse_classic(&ctx);

	free(tokenization);

	return result;
}

struct dulcet_parse_result dulcet_parse_de_bruijn(const char *input, unsigned int input_len)
{
	struct tokenization *tokenization = malloc(
		sizeof(struct tokenization) + TOKENIZATION_BUFFER_MAX_SIZE * sizeof(struct token));
	tokenization->size = 0;

	struct sorvete_sv input_sv = sorvete_sv_from_parts(input, input_len);

	int rc = __dulcet_tokenize(tokenization, input_sv);
	if (rc != 0) {
		if (tokenization->size > 0) {
			struct token last_tk = tokenization->buf[tokenization->size - 1];

			return __dulcet_error(DULCET_PARSE_ERROR_CAUSE_UNKNOWN, last_tk);
		} else {
			// FIXME: indicate in some way that there is not valid position
			return (struct dulcet_parse_result) {
                                .kind = DULCET_PARSE_ERROR,
                                .error = {
                                        .cause = DULCET_PARSE_ERROR_CAUSE_UNKNOWN,
                                        .text_start = 0,
                                        .text_len = 0,
                                        .line = 0,
                                        .column = 0,
                                },
                        };
		}
	}

	struct parsing_context ctx = {
                .tks = {
		        .tokenization = tokenization,
		        .pos = 0,
	        },
                .paren_depth = 0,
                .parameter_stack_size = 0,
        };

	struct dulcet_parse_result result = __dulcet_parse_de_bruijn(&ctx);

	free(tokenization);

	return result;
}
