/*
 * Copyright (c) 2022 Leonardo Duarte
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef _DULCET_PARSER_H
#define _DULCET_PARSER_H

#include "dulcet.h"

enum dulcet_parse_error_cause {
	DULCET_PARSE_ERROR_CAUSE_UNKNOWN,
	DULCET_PARSE_ERROR_CAUSE_UNEXPECTED_TOKEN,
	DULCET_PARSE_ERROR_CAUSE_UNMATCHED_PAREN,

	// FIXME: perhaps unbound variables should simply be treated
	//        as free variables, and not as errors
	DULCET_PARSE_ERROR_CAUSE_UNBOUND_VARIABLE,
};

struct dulcet_parse_error {
	enum dulcet_parse_error_cause cause;
	const char *text_start;
	unsigned int text_len;
	unsigned int line;
	unsigned int column;
};

struct dulcet_parse_result {
	enum { DULCET_PARSE_ERROR, DULCET_PARSE_OK } kind;

	union {
		struct dulcet_term *value;
		struct dulcet_parse_error error;
	};
};

struct dulcet_parse_result dulcet_parse_classic(const char *input, unsigned int input_len);

struct dulcet_parse_result dulcet_parse_de_bruijn(const char *input, unsigned int input_len);

#endif // _DULCET_PARSER_H
