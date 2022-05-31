/*
 * Copyright (c) 2022 Leonardo Duarte
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "dulcet.h"

#include "dulcet_parser.h"

static void print_usage(const char *program_name)
{
	printf("dulceti: an interpreter for the untyped lambda calculus\n");
	printf("\n");
	printf("Usage: %s [options]\n", program_name);
	printf("Options:\n");
	printf("  -h, --help           \tDisplay this information.\n");
	printf("  -f <input_file_path> \tRun the interpreter on the given file, which may be `-` for stdin.\n");
	printf("                       \tBy default, the interpreter will accept input from stdin.\n");
	printf("  -o <output_file_path>\tRun the interpreter and write its output to the given file, creating it if doesn't exist and overriding its contents.\n");
	printf("                       \tBy default, the interpreter will write its output to stdout.\n");
}

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
	FILE *output_fp = stdout;

	while (argc > 0) {
		char *opt = shift_arg(&argc, &argv);

		if (strcmp(opt, "-h") == 0 || strcmp(opt, "--help") == 0) {
			print_usage(program_name);

			return 0;
		} else if (strcmp(opt, "-f") == 0) {
			if (argc <= 0) {
				fprintf(stderr,
					"%s: fatal error: `-f` flag requires a file argument\n",
					program_name);
				return 1;
			}

			char *input_file_path = shift_arg(&argc, &argv);

			if (strcmp(input_file_path, "-") == 0) {
				input_fp = stdin;
			} else {
				input_fp = fopen(input_file_path, "r");
				if (!input_fp) {
					fprintf(stderr,
						"%s: fatal error: could not open file `%s`: %s\n",
						program_name, input_file_path, strerror(errno));
					return 1;
				}
			}
		} else if (strcmp(opt, "-o") == 0) {
			if (argc <= 0) {
				fprintf(stderr,
					"%s: fatal error: `-o` flag requires a file argument\n",
					program_name);
				return 1;
			}

			char *output_file_path = shift_arg(&argc, &argv);

			output_fp = fopen(output_file_path, "w");
			if (!output_fp) {
				fprintf(stderr, "%s: fatal error: could not open file `%s`: %s\n",
					program_name, output_file_path, strerror(errno));
				return 1;
			}
		} else {
			fprintf(stderr, "%s: fatal error: unknown parameter `%s`\n", program_name,
				opt);
			return 1;
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
