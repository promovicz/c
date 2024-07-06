/*
 * cplr - Utility for running C code
 *
 * Copyright (C) 2020-2021 Ingo Albrecht <copyright@promovicz.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CPLR_STRING_H
#define CPLR_STRING_H

#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#include "attr.h"

extern char *xstrdup(char *s);

extern bool strprefix(const char *str, const char *pfx);
extern bool strsuffix(const char *str, const char *suf);

CEXT_FUNC_FORMAT(printf, 2, 0)
CEXT_FUNC_ARG_FORMAT(2)
extern char *vmsnprintf(size_t limit, const char *fmt, va_list a);

CEXT_FUNC_FORMAT(printf, 2, 3)
CEXT_FUNC_ARG_FORMAT(2)
extern char *msnprintf(size_t limit, const char *fmt, ...);

CEXT_FUNC_FORMAT(printf, 1, 0)
CEXT_FUNC_ARG_FORMAT(1)
extern char *vmsprintf(const char *fmt, va_list a);


CEXT_FUNC_FORMAT(printf, 1, 2)
CEXT_FUNC_ARG_FORMAT(1)
extern char *msprintf(const char *fmt, ...);

#endif /* !CPLR_STRING_H */
