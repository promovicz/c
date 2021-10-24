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

#include "cext/memory.h"

#include "cplr.h"

cplr_t *cplr_new(void) {
  cplr_t *res = xcalloc(sizeof(cplr_t), 1);

  res->t_env = CPLR_ENV_POSIX;

  return res;
}

void cplr_free(cplr_t *c) {
  if(c->tcc) {
    tcc_delete(c->tcc);
    c->tcc = NULL;
  }
  lptrfree((void*)&c->out);
  lptrfree((void*)&c->g_codebuf);
  lptrfree((void*)&c->g_dumpbuf);
  l_clear(&c->defdef);
  l_clear(&c->defsys);
  l_clear(&c->sysdirs);
  l_clear(&c->incdirs);
  l_clear(&c->libdirs);
  l_clear(&c->defs);
  l_clear(&c->syss);
  l_clear(&c->incs);
  l_clear(&c->libs);
  l_clear(&c->pkgs);
  l_clear(&c->srcs);
  l_clear(&c->tlfs);
  l_clear(&c->stms);
  l_clear(&c->befs);
  l_clear(&c->afts);
  xfree(c);
}

cplr_t *cplr_clone(cplr_t *c) {
  cplr_t *r = cplr_new();
  r->argc = c->argc;
  r->argv = c->argv;
  r->argp = c->argp;
  r->flag = c->flag;
  r->t_env = c->t_env;
  r->out = NULL;
  r->g_codebuf = NULL;
  r->g_dumpbuf = NULL;
  l_clone(&c->defdef, &r->defdef);
  l_clone(&c->defsys, &r->defsys);
  l_clone(&c->sysdirs, &r->sysdirs);
  l_clone(&c->incdirs, &r->incdirs);
  l_clone(&c->libdirs, &r->libdirs);
  l_clone(&c->defs, &r->defs);
  l_clone(&c->syss, &r->syss);
  l_clone(&c->incs, &r->incs);
  l_clone(&c->libs, &r->libs);
  l_clone(&c->pkgs, &r->pkgs);
  l_clone(&c->srcs, &r->srcs);
  l_clone(&c->tlfs, &r->tlfs);
  l_clone(&c->stms, &r->stms);
  l_clone(&c->befs, &r->befs);
  l_clone(&c->afts, &r->afts);
  return r;
}
