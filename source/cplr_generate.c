
/* bar for separating the dump - 80 chars */
static const char *bar =
  "========================================"
  "========================================";

static void cplr_emit(cplr_t *c,
		      cplr_gstate_t nstate,
		      const char * file, int line,
		      const char *fmt, ...) {
  bool needline = false;
  va_list a;
  if(nstate == CPLR_GSTATE_COMMENT) {
    needline = false;
  } else if(c->g_state != nstate) {
    needline = true;
  } else if(c->g_prevfile == NULL || strcmp(c->g_prevfile, file) != 0) {
    needline = true;
  } else if(nstate == CPLR_GSTATE_PREPROC) {
    needline = false;
  } else {
    needline = (c->g_prevline && (line != (c->g_prevline+1)));
  }
  va_start(a, fmt);
  if(c->g_code) {
    if(needline)
      fprintf(c->g_code, "#line %d \"%s\"\n", line, file);
    vfprintf(c->g_code, fmt, a);
  }
  va_end(a);
  va_start(a, fmt);
  if(c->g_dump) {
    if(needline && (c->flag & CPLR_FLAG_VERBOSE))
      fprintf(c->g_dump, "#line %d \"%s\"\n", line, file);
    vfprintf(c->g_dump, fmt, a);
  }
  va_end(a);
  if(nstate != CPLR_GSTATE_COMMENT) {
    c->g_state = nstate;
    c->g_prevline = line;
    if(c->g_prevfile) {
      xptrfree((void**)&c->g_prevfile);
    }
    c->g_prevfile = strdup(file);
  }
}

#define CPLR_EMIT_COMMENT(c, fmt, ...)			\
  cplr_emit(c, CPLR_GSTATE_COMMENT, NULL,		\
	    1, "/* " fmt " */\n", ##__VA_ARGS__)
#define CPLR_EMIT_PREPROC(c, fn, fmt, ...)		\
  cplr_emit(c, CPLR_GSTATE_PREPROC, fn,			\
	    1, fmt, ##__VA_ARGS__)
#define CPLR_EMIT_TOPLEVEL(c, fn, fmt, ...)     \
  cplr_emit(c, CPLR_GSTATE_TOPLEVEL, fn,	\
	    1, fmt, ##__VA_ARGS__)
#define CPLR_EMIT_INTERNAL(c, fmt, ...)			\
  cplr_emit(c, CPLR_GSTATE_INTERNAL, "internal",	\
	    __LINE__, fmt, ##__VA_ARGS__)
#define CPLR_EMIT_STATEMENT(c, fn, fmt, ...)    \
  cplr_emit(c, CPLR_GSTATE_STATEMENT, fn,       \
	    1, fmt, ##__VA_ARGS__)

static void cplr_emit_minilib(cplr_t *c, const char *phase, const char *mlb, int count) {
  if(c->flag & CPLR_FLAG_VERBOSE) {
    fprintf(stderr, "Emitting minilib '%s' phase '%s'\n", mlb, phase);
  }
  char *fn = msprintf("%s_mlib_%d", phase, count);
  CPLR_EMIT_PREPROC(c, fn, "#define minilib_%s\n", phase);
  CPLR_EMIT_PREPROC(c, fn, "#include \"%s.m\"\n", mlb);
  CPLR_EMIT_PREPROC(c, fn, "#undef minilib_%s\n", phase);
  ptrfree((void**)&fn);
}

static void cplr_emit_minilibs(cplr_t *c, const char *phase, bool reverse) {
  int i;
  ln_t *n;
  if(reverse) {
    i = l_size(&c->mlbs);
    L_BACKWARDS(&c->mlbs, n) {
      cplr_emit_minilib(c, phase, n->v.s, i--);
    }
  } else {
    i = 0;
    L_FORWARD(&c->mlbs, n) {
      cplr_emit_minilib(c, phase, n->v.s, i++);
    }
  }
}

static int cplr_generate_code(cplr_t *c) {
  int i;
  ln_t *n;
  char fn[32];
  bool minilibs = !l_empty(&c->mlbs);
  if(minilibs || !l_empty(&c->defsys)) {
    CPLR_EMIT_COMMENT(c, "defsysinclude");
    i = 0;
    L_FORWARD(&c->defsys, n) {
      snprintf(fn, sizeof(fn), "defsysinclude_%d", i++);
      CPLR_EMIT_PREPROC(c, fn, "#include <%s>\n", n->v.s);
    }
  }
  if(minilibs || !l_empty(&c->syss)) {
    CPLR_EMIT_COMMENT(c, "sysinclude");
    cplr_emit_minilibs(c, "sysinclude", false);
    i = 0;
    L_FORWARD(&c->syss, n) {
      snprintf(fn, sizeof(fn), "sysinclude_%d", i++);
      CPLR_EMIT_PREPROC(c, fn, "#include <%s>\n", n->v.s);
    }
  }
  if(minilibs || !l_empty(&c->incs)) {
    CPLR_EMIT_COMMENT(c, "include");
    cplr_emit_minilibs(c, "include", false);
    i = 0;
    L_FORWARD(&c->incs, n) {
      snprintf(fn, sizeof(fn), "include_%d", i++);
      CPLR_EMIT_PREPROC(c, fn, "#include \"%s\"\n", n->v.s);
    }
  }
  if(minilibs || !l_empty(&c->tlfs)) {
    CPLR_EMIT_COMMENT(c, "toplevel");
    cplr_emit_minilibs(c, "toplevel", false);
    i = 0;
    L_FORWARD(&c->tlfs, n) {
      snprintf(fn, sizeof(fn), "toplevel_%d", i++);
      CPLR_EMIT_TOPLEVEL(c, fn, "%s;\n", n->v.s);
    }
  }
  CPLR_EMIT_COMMENT(c, "main");
  CPLR_EMIT_INTERNAL(c, "int main(int argc, char **argv) {\n");
  CPLR_EMIT_INTERNAL(c, "    int ret = 0;\n");
  if(minilibs || !l_empty(&c->befs)) {
    CPLR_EMIT_COMMENT(c, "before");
    cplr_emit_minilibs(c, "before", false);
    i = 0;
    L_FORWARD(&c->befs, n) {
      snprintf(fn, sizeof(fn), "before_%d", i++);
      CPLR_EMIT_STATEMENT(c, fn, "    %s;\n", n->v.s);
    }
  }
  if(minilibs || !l_empty(&c->stms)) {
    CPLR_EMIT_COMMENT(c, "statement");
    cplr_emit_minilibs(c, "statement", false);
    i = 0;
    L_FORWARD(&c->stms, n) {
      snprintf(fn, sizeof(fn), "statement_%d", i++);
      CPLR_EMIT_STATEMENT(c, fn, "    %s;\n", n->v.s);
    }
  }
  if(minilibs || !l_empty(&c->afts)) {
    CPLR_EMIT_COMMENT(c, "after");
    i = l_size(&c->afts);
    L_BACKWARDS(&c->afts, n) {
      snprintf(fn, sizeof(fn), "after_%d", i--);
      CPLR_EMIT_STATEMENT(c, fn, "    %s;\n", n->v.s);
    }
    cplr_emit_minilibs(c, "after", true);
  }
  CPLR_EMIT_COMMENT(c, "done");
  CPLR_EMIT_INTERNAL(c, "    return ret;\n");
  CPLR_EMIT_INTERNAL(c, "}\n");
  return 0;
}

#ifdef _GNU_SOURCE
static ssize_t code_stream_write(void *cp, const char *buf, size_t size) {
  cplr_t *c = (cplr_t*)cp;
  assert(c->g_codebuf);
  assert(size < SSIZE_MAX);
  size_t ol = strlen(c->g_codebuf);
  size_t nl = ol + size + 1;
  char *n = xrealloc(c->g_codebuf, nl);
  c->g_codebuf = n;
  strncpy(n + ol, buf, size + 1);
  return size;
}
static cookie_io_functions_t code_stream_functions = {
  NULL, &code_stream_write, NULL, NULL
};
static ssize_t dump_stream_write(void *cp, const char *buf, size_t size) {
  cplr_t *c = (cplr_t*)cp;
  assert(c->g_dumpbuf);
  assert(size < SSIZE_MAX);
  size_t ol = strlen(c->g_dumpbuf);
  size_t nl = ol + size + 1;
  char *n = xrealloc(c->g_dumpbuf, nl);
  c->g_dumpbuf = n;
  strncpy(n + ol, buf, size + 1);
  return size;
}
static cookie_io_functions_t dump_stream_functions = {
  NULL, &dump_stream_write, NULL, NULL
};
#endif

static void cplr_generate_open(cplr_t *c) {
#ifdef _GNU_SOURCE
  c->g_codebuf = strdup("");
  c->g_code = fopencookie(c, "w", code_stream_functions);
#else
  c->g_codebuf = xcalloc(2^16, 1);
  c->g_code = fmemopen(c->g_codebuf, 2^16, "w");
#endif
  if(c->flag & CPLR_FLAG_DUMP) {
#ifdef _GNU_SOURCE
    c->g_dumpbuf = strdup("");
    c->g_dump = fopencookie(c, "w", dump_stream_functions);
#else
    c->g_dumpbuf = xcalloc(2^16, 1);
    c->g_dump = fmemopen(c->g_dumpbuf, 2^16, "w");
#endif
  }
}

static void cplr_generate_close(cplr_t *c) {
 fclose(c->g_code);
  c->g_code = NULL;
  if(c->flag & CPLR_FLAG_DUMP) {
    fflush(c->g_dump);
    fclose(c->g_dump);
    c->g_dump = NULL;
  }
}

static void cplr_generate_dump(cplr_t *c) {
  if(c->flag & CPLR_FLAG_DUMP) {
    fprintf(stderr, "%s\n", bar);
    fflush(stderr);
    size_t total = strlen(c->g_dumpbuf);
    char *buf = c->g_dumpbuf;
    FILE *dumpout = popen("cat -n - 2>&1", "w");
    size_t done = 0;
    while(done < total) {
      size_t step = total - done;
      if(step > sizeof(buf)) {
	step = sizeof(buf);
      }
      size_t n = fwrite(buf + done, 1, step, dumpout);
      if(n < step) {
	perror("fwrite");
	exit(1);
      }
      done += n;
    }
    pclose(dumpout);
    fprintf(stderr, "%s\n", bar);
    fflush(stderr);
  }
}

static void cplr_generate_report(cplr_t *c) {
 if(c->flag & CPLR_FLAG_VERBOSE) {
    size_t cl = 0, dl = 0;
    if(c->g_codebuf)
      cl = strlen(c->g_codebuf);
    if(c->g_dumpbuf)
      dl = strlen(c->g_dumpbuf);
    fprintf(stderr, "Generated bytes: %zu code, %zu dump\n", cl, dl);
  }
}

int cplr_generate(cplr_t *c) {
  /* say hello */
  if(c->flag & CPLR_FLAG_VERBOSE) {
    fprintf(stderr, "Generating code\n");
  }
  /* alloc buffers and open streams */
  cplr_generate_open(c);
  /* perform code generation */
  cplr_generate_code(c);
  /* flush and close */
  cplr_generate_close(c);
  /* emit the dump buffer */
  cplr_generate_dump(c);
  /* report stats */
  cplr_generate_report(c);
  /* done */
  return 0;
}