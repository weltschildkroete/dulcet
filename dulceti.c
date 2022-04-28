/*
 * Copyright (c) 2022 Leonardo Duarte
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>
#include <string.h>

#include "dulcet.h"

#include "dulcet_parser.h"

static char *shift_arg(int *argc, char ***argv)
{
	char *arg = **argv;
	*argc -= 1;
	*argv += 1;
	return arg;
}

int main(int argc, char **argv)
{
	char *program_name = shift_arg(&argc, &argv);
	FILE *input_fp = stdin;

	while (argc > 0) {
		char *arg = shift_arg(&argc, &argv);

		if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
			printf("%s: an interpreter for the untyped lambda calculus\n",
			       program_name);

			return 0;
		}
	}

	char input[BUFSIZ] = { 0 };

	int bytes_read = fread(input, sizeof(char), BUFSIZ, input_fp);
	(void) bytes_read;
	int error_code = ferror(input_fp);
	if (error_code != 0) {
		// FIXME: strerror(ferror) is most probably incorrect
		perror(strerror(error_code));
		return 1;
	}

	struct dulcet_term *input_term = dulcet_parse_de_bruijn(input, 0);

	dulcet_beta_nor(input_term);

	dulcet_term_print_de_bruijn(input_term);

	dulcet_term_free(input_term);

	return 0;
}
