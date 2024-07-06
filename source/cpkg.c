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

#include <stdio.h>
#include <stdlib.h>

#include <cext/memory.h>
#include <cext/string.h>

#include "cpkg.h"

bool cpkg_exists(const char *name, bool verbose) {
  int res;
  bool ret = false;
  char *cmd = msprintf("pkg-config --exists %s", name);
  if(verbose) {
    fprintf(stderr, "Running \"%s\"\n", cmd);
  }
  res = system(cmd);
  if(res == -1 || res == 127) {
    fprintf(stderr, "Error: Could not execute \"%s\"\n", cmd);
  } else if(res) {
    fprintf(stderr, "Error: Package %s not present\n", name);
  } else {
    ret = true;
  }
  cext_free(cmd);
  return ret;
}

char *cpkg_retrieve(const char *name, const char *what, bool verbose) {
  int res;
  FILE *ps;
  char *cmd;
  char rbuf[1024];
  cmd = msprintf("pkg-config %s %s", what, name);
  if(verbose) {
    fprintf(stderr, "Running \"%s\"\n", cmd);
  }
  ps = popen(cmd, "r");
  if(!ps) {
    fprintf(stderr, "Error: Could not popen \"%s\"\n", cmd);
    goto err_popen;
  }
  res = fread(rbuf, 1, sizeof(rbuf), ps);
  if(res < 0) {
    fprintf(stderr, "Error: Failed to read from \"%s\"\n", cmd);
    goto err_fread;
  }
  if(res == sizeof(rbuf)) {
    fprintf(stderr, "Error: Package options for %s are too long.\n", name);
    goto err_fread;
  }
  rbuf[res] = 0;
  if(rbuf[res - 1] == '\n')
    rbuf[res - 1] = 0;
  pclose(ps);
  cext_free(cmd);
  return strdup(rbuf);
 err_fread:
  pclose(ps);
 err_popen:
  cext_free(cmd);
  return NULL;
}
