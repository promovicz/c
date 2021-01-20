
#include <getopt.h>

#include "cplr.h"

#define USE_GETOPT_LONG

/* short options */
const char *shortopts = "-:hHVvdnpD:U:I:i:S:s:L:l:M:m:P:e:b:a:t:o:f:-";

/* long is optional */
#ifdef USE_GETOPT_LONG
/* long options */
const struct option longopts[] = {
  {"help",     0, NULL, 'h'},
  {"herald",   0, NULL, 'H'},
  {"version",  0, NULL, 'V'},

  {"verbose",  0, NULL, 'v'},
  {"dump",     0, NULL, 'd'},
  {"noexec",   0, NULL, 'n'},
  {"pristine", 0, NULL, 'p'},

  {NULL,    1, NULL, 'D'},
  {NULL,    1, NULL, 'U'},
  {NULL,    1, NULL, 'I'},
  {NULL,    1, NULL, 'i'},
  {NULL,    1, NULL, 'S'},
  {NULL,    1, NULL, 's'},

  {NULL,    1, NULL, 'L'},
  {NULL,    1, NULL, 'l'},

  {NULL,    1, NULL, 'M'},
  {NULL,    1, NULL, 'm'},

  {NULL,    1, NULL, 'P'},

  {NULL,    1, NULL, 'e'},
  {NULL,    1, NULL, 'b'},
  {NULL,    1, NULL, 'a'},
  {NULL,    1, NULL, 't'},

  {NULL,    1, NULL, 'o'},
  {NULL,    1, NULL, 'f'},

  {NULL,    0, NULL, '-'},

  {NULL,    0, NULL, 0 },
};
/* help strings for long options */
const char *longhelp[] = {
  "show help message",
  "show herald message",
  "show version string",

  "verbose cplr output",
  "dump generated code",
  "do not run, just compile",
  "use pristine environment",

  "define cpp symbol",
  "undefine cpp symbol",
  "add include directory",
  "add include",
  "add sysinclude directory",
  "add sysinclude",

  "add library directory",
  "add library",

  "add minilib directory",
  "add minilib",

  "add pkg-config library",

  "add main statement",
  "add before statement",
  "add after statement",
  "add toplevel statement",

  "output object file instead of executing",
  "add file (source, object, archive)",

  "begin program arguments",
  NULL,
};
#endif /* USE_GETOPT_LONG */


static void cplr_show_summary(cplr_t *c, FILE*out) {
  fprintf(out, "Usage: %s [-vdnphHV] <statement>...\n", c->argv[0]);
  fprintf(out, "The C piler: a tool for executing C code\n\n");
}

/* help function */
static void cplr_show_help(cplr_t *c, FILE *out) {
  /* short summary */
  cplr_show_summary(c, out);
  /* with getopt_long we can easily print more information */
#ifdef USE_GETOPT_LONG
  int i;
  for(i = 0; longhelp[i]; i++) {
    if(longopts[i].name) {
      fprintf(out, "  -%c, --%-10s\t%s\n",
	      (char)longopts[i].val,
	      longopts[i].name,
	      longhelp[i]);
    } else {
      fprintf(out, "  -%c\t\t\t%s\n",
	      (char)longopts[i].val,
	      longhelp[i]);
    }
  }
  fprintf(out, "\n");
#endif
}

/* herald function */
static void cplr_show_herald(cplr_t *c, FILE *out) {
  /* program basename for mail address */
  const char *bn = basename(c->argv[0]);

  /* short summary */
  cplr_show_summary(c, out);

  /* history */
  fprintf(out, "Invented around the ides of October anno MMXX.\n\n");

  /* blessing */
  fprintf(out, "May this be as useful for you as it is for me.\n\n");

  /* copyright information */
  fprintf(out, "Copyright (C) 2020-2021 Ingo Albrecht <cplr@promovicz.org>.\n");
  fprintf(out, "Licensed under the GNU General Public License version 3 or later.\n");
  fprintf(out, "See package file COPYING or https://www.gnu.org/licenses/.\n\n");
}

/* version function */
static void cplr_show_version(cplr_t *c, FILE *out) {
}

static void cplr_optparse_main(cplr_t *c, char *arg) {
  lh_t *lh = NULL;
  switch(c->argt) {
  case CPLR_MAINOPT_STATEMENT:
    lh = &c->stms;
    break;
  case CPLR_MAINOPT_BEFORE:
    lh = &c->befs;
    break;
  case CPLR_MAINOPT_AFTER:
    lh = &c->afts;
    break;
  case CPLR_MAINOPT_TOPLEVEL:
    lh = &c->tlfs;
    break;
  case CPLR_MAINOPT_FILE:
    lh = &c->srcs;
    break;
  default:
    break;
  }
  if(lh) {
    l_append_str_static(lh, optarg);
  }
}

/* option parser */
int cplr_optparse(cplr_t *c, int argc, char **argv) {
  int opt;

  /* remember argc/argv */
  c->argc = argc;
  c->argv = argv;

  /* initial main arg is statement */
  c->argt = CPLR_MAINOPT_STATEMENT;

  /* parse options */
  optind = 1;
  opterr = 0;
  while(1) {
    /* get next option */
#ifdef USE_GETOPT_LONG
    opt = getopt_long(argc, argv, shortopts, longopts, NULL);
#else
    opt = getopt(argc, argv, shortopts);
#endif

    /* check if finished */
    if (opt == -1)
      goto done;

    /* dispatch the option */
    switch(opt) {
      /* internal */
    case 0: /* handled by getopt */
      break;
    case 1: /* non-option arguments */
      cplr_optparse_main(c, optarg);
      break;

      /* information */
    case 'h': /* help requested */
      goto help;
    case 'H': /* herald requested */
      goto herald;
    case 'V': /* version requested */
      goto version;

      /* flags */
    case 'v': /* enable verbose */
      c->flag |= CPLR_FLAG_VERBOSE;
      break;
    case 'd': /* enable dump */
      c->flag |= CPLR_FLAG_DUMP;
      break;
    case 'n': /* enable norun */
      c->flag |= CPLR_FLAG_NORUN;
      break;
    case 'p': /* enable pristine */
      c->flag |= CPLR_FLAG_PRISTINE;
      break;

      /* preprocessor */
    case 'D':
      l_append_str_owned(&c->defs, msnprintf(1024, "-D%s", optarg));
      break;
    case 'U':
      l_append_str_owned(&c->defs, msnprintf(1024, "-U%s", optarg));
      break;
    case 'I':
      l_append_str_static(&c->incdirs, optarg);
      break;
    case 'i':
      l_append_str_static(&c->incs, optarg);
      break;
    case 'S':
      l_append_str_static(&c->sysdirs, optarg);
      break;
    case 's':
      l_append_str_static(&c->syss, optarg);
      break;

      /* libraries */
    case 'L':
      l_append_str_static(&c->libdirs, optarg);
      break;
    case 'l':
      l_append_str_static(&c->libs, optarg);
      break;

      /* minilibs */
    case 'M':
      l_append_str_static(&c->mlbdirs, optarg);
      break;
    case 'm':
      l_append_str_static(&c->mlbs, optarg);
      break;

      /* pkg-config */
    case 'P':
      l_append_str_static(&c->pkgs, optarg);
      break;

      /* statements */
    case 'e':
      if(strcmp(":", optarg) == 0) {
	c->argt = CPLR_MAINOPT_STATEMENT;
      } else {
	l_append_str_static(&c->stms, optarg);
      }
      break;
    case 'b':
      if(strcmp(":", optarg) == 0) {
	c->argt = CPLR_MAINOPT_BEFORE;
      } else {
	l_append_str_static(&c->befs, optarg);
      }
      break;
    case 'a':
      if(strcmp(":", optarg) == 0) {
	c->argt = CPLR_MAINOPT_AFTER;
      } else {
	l_append_str_static(&c->afts, optarg);
      }
      break;
    case 't':
      if(strcmp(":", optarg) == 0) {
	c->argt = CPLR_MAINOPT_TOPLEVEL;
      } else {
	l_append_str_static(&c->tlfs, optarg);
      }
      break;

      /* output file */
    case 'o':
      break;

      /* input files */
    case 'f':
      if(strcmp(":", optarg) == 0) {
	c->argt = CPLR_MAINOPT_FILE;
      } else {
	l_append_str_static(&c->srcs, optarg);
      }
      break;

      /* program arguments */
    case '-':
      goto done;

      /* errors */
    case ':': /* missing option argument */
      fprintf(stderr, "Missing argument for option -%c\n", optopt);
      goto err;
    case '?': /* unknown option */
      fprintf(stderr, "Unknown option -%c\n", optopt);
      goto err;
    default:
      fprintf(stderr, "Option processing problem\n");
      goto err;
    }
  }

 done:
  /* set index of program args */
  c->argp = optind;
  return 0;

 err:
  return 1;

 help:
  /* show help */
  cplr_show_help(c, stdout);
  return 2;
 herald:
  /* show herald */
  cplr_show_herald(c, stdout);
  return 2;
 version:
  /* show version */
  cplr_show_version(c, stdout);
  return 2;
}
