/*
 * Copyright (c) 2022 Leonardo Duarte
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef _DULCET_PARSER_H
#define _DULCET_PARSER_H

#include "dulcet.h"

struct dulcet_term *dulcet_parse_de_bruijn(const char *input, unsigned int input_len);

#endif // _DULCET_PARSER_H
