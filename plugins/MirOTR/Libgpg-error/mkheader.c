/* mkheader.c - Create a header file for libgpg-error
 * Copyright (C) 2010 Free Software Foundation, Inc.
 * Copyright (C) 2014 g10 Code GmbH
 *
 * This file is free software; as a special exception the author gives
 * unlimited permission to copy and/or distribute it, with or without
 * modifications, as long as this notice is preserved.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define PGM "mkheader"

#define LINESIZE 1024

static const char *host_os;
static const char *host_triplet;
static char *srcdir;
static const char *hdr_version;
static const char *hdr_version_number;

/* Values take from the supplied config.h.  */
static int have_stdint_h;
static int have_w32_system;
static int have_w64_system;
static char *replacement_for_off_type;

/* Various state flags.  */
static int stdint_h_included;


/* The usual free wrapper.  */
static void
xfree (void *a)
{
  if (a)
    free (a);
}


static char *
xstrdup (const char *string)
{
  char *p;

  p = malloc (strlen (string)+1);
  if (!p)
    {
      fputs (PGM ": out of core\n", stderr);
      exit (1);
    }
  strcpy (p, string);
  return p;
}


/* Parse the supplied config.h file and extract required info.
   Returns 0 on success.  */
static int
parse_config_h (const char *fname)
{
  FILE *fp;
  char line[LINESIZE];
  int lnr = 0;
  char *p1;

  fp = fopen (fname, "r");
  if (!fp)
    {
      fprintf (stderr, "%s:%d: can't open file: %s",
               fname, lnr, strerror (errno));
      return 1;
    }

  while (fgets (line, LINESIZE, fp))
    {
      size_t n = strlen (line);

      lnr++;
      if (!n || line[n-1] != '\n')
        {
          fprintf (stderr,
                   "%s:%d: trailing linefeed missing, line too long or "
                   "embedded nul character\n", fname, lnr);
          break;
        }
      line[--n] = 0;

      if (strncmp (line, "#define ", 8))
        continue; /* We are only interested in define lines.  */
      p1 = strtok (line + 8, " \t");
      if (!*p1)
        continue; /* oops */
      if (!strcmp (p1, "HAVE_STDINT_H"))
        have_stdint_h = 1;
      else if (!strcmp (p1, "HAVE_W32_SYSTEM"))
        have_w32_system = 1;
      else if (!strcmp (p1, "HAVE_W64_SYSTEM"))
        have_w64_system = 1;
      else if (!strcmp (p1, "REPLACEMENT_FOR_OFF_T"))
        {
          p1 = strtok (NULL, "\"");
          if (!*p1)
            continue; /* oops */
          xfree (replacement_for_off_type);
          replacement_for_off_type = xstrdup (p1);
        }
    }

  if (ferror (fp))
    {
      fprintf (stderr, "%s:%d: error reading file: %s\n",
               fname, lnr, strerror (errno));
      return 1;
    }

  fclose (fp);
  return 0;
}


/* Write LINE to stdout.  The function is allowed to modify LINE.  */
static void
write_str (char *line)
{
  if (fputs (line, stdout) == EOF)
    {
      fprintf (stderr, PGM ": error writing to stdout: %s\n", strerror (errno));
      exit (1);
    }
}

static void
write_line (char *line)
{
  if (puts (line) == EOF)
    {
      fprintf (stderr, PGM ": error writing to stdout: %s\n", strerror (errno));
      exit (1);
    }
}


/* Write SOURCE or CODES line to stdout.  The function is allowed to
   modify LINE.  Trailing white space is already removed.  Passing
   NULL resets the internal state.  */
static void
write_sources_or_codes (char *line)
{
  static int in_intro;
  char *p1, *p2;

  if (!line)
    {
      in_intro = 1;
      return;
    }

  if (!*line)
    return;

  if (in_intro)
    {
      if (!strchr ("0123456789", *line))
        return;
      in_intro = 0;
    }

  p1 = strtok (line, " \t");
  p2 = p1? strtok (NULL, " \t") : NULL;

  if (p1 && p2 && strchr ("0123456789", *p1) && *p2)
    {
      write_str ("    ");
      write_str (p2);
      write_str (" = ");
      write_str (p1);
      write_str (",\n");
    }
}


/* Write system errnos to stdout.  The function is allowed to
   modify LINE.  Trailing white space is already removed.  Passing
   NULL resets the internal state.  */
static void
write_errnos_in (char *line)
{
  static int state;
  char *p1, *p2;

  if (!line)
    {
      state = 0;
      return;
    }

  if (!*line)
    return;

  if (!state && strchr ("0123456789", *line))
    state = 1;
  else if (state == 1 && !strchr ("0123456789", *line))
    state = 2;

  if (state != 1)
    return;

  p1 = strtok (line, " \t");
  p2 = p1? strtok (NULL, " \t") : NULL;

  if (p1 && p2 && strchr ("0123456789", *p1) && *p2)
    {
      write_str ("    GPG_ERR_");
      write_str (p2);
      write_str (" = GPG_ERR_SYSTEM_ERROR | ");
      write_str (p1);
      write_str (",\n");
    }
}


/* Create the full file name for NAME and return a newly allocated
   string with it.  If name contains a '&' and REPL is not NULL
   replace '&' with REPL. */
static char *
mk_include_name (const char *name, const char *repl)
{
  FILE *fp;
  char *incfname, *p;
  const char *s;

  incfname = malloc (strlen (srcdir) + strlen (name)
                     + (repl?strlen (repl):0) + 1);
  if (!incfname)
    {
      fputs (PGM ": out of core\n", stderr);
      exit (1);
    }

  if (*name == '.' && name[1] == '/')
    *incfname = 0;
  else
    strcpy (incfname, srcdir);
  p = incfname + strlen (incfname);
  for (s=name; *s; s++)
    {
      if (*s == '&' && repl)
        {
          while (*repl)
            *p++ = *repl++;
          repl = NULL;  /* Replace only once.  */
        }
      else
        *p++ = *s;
    }
  *p = 0;
  return incfname;
}


/* Include the file NAME from the source directory.  The included file
   is not further expanded.  It may have comments indicated by a
   double hash mark at the begin of a line.  OUTF is called for each
   read line and passed a buffer with the content of line sans line
   line endings.  If NAME is prefixed with "./" it is included from
   the current directory and not from the source directory. */
static void
include_file (const char *fname, int lnr, const char *name, void (*outf)(char*))
{
  FILE *fp;
  char *incfname;
  int inclnr;
  char line[LINESIZE];
  int repl_flag;

  repl_flag = !!strchr (name, '&');
  incfname = mk_include_name (name, repl_flag? host_triplet : NULL);
  fp = fopen (incfname, "r");
  if (!fp && repl_flag)
    {
      /* Try again using the OS string.  */
      free (incfname);
      incfname = mk_include_name (name, host_os);
      fp = fopen (incfname, "r");
    }
  if (!fp)
    {
      fprintf (stderr, "%s:%d: error including `%s': %s\n",
               fname, lnr, incfname, strerror (errno));
      exit (1);
    }

  if (repl_flag)
    fprintf (stderr,"%s:%d: note: including '%s'\n",
             fname, lnr, incfname);

  inclnr = 0;
  while (fgets (line, LINESIZE, fp))
    {
      size_t n = strlen (line);

      inclnr++;
      if (!n || line[n-1] != '\n')
        {
          fprintf (stderr,
                   "%s:%d: trailing linefeed missing, line too long or "
                   "embedded nul character\n", incfname, inclnr);
          fprintf (stderr,"%s:%d: note: file '%s' included from here\n",
                   fname, lnr, incfname);
          exit (1);
        }
      line[--n] = 0;
      while (line[n] == ' ' || line[n] == '\t' || line[n] == '\r')
        {
          line[n] = 0;
          if (!n)
            break;
          n--;
        }

      if (line[0] == '#' && line[1] == '#')
        {
          if (!strncmp (line+2, "EOF##", 5))
            break; /* Forced EOF.  */
        }
      else
        outf (line);
    }
  if (ferror (fp))
    {
      fprintf (stderr, "%s:%d: error reading `%s': %s\n",
               fname, lnr, incfname, strerror (errno));
      exit (1);
    }
  fclose (fp);
  free (incfname);
}


/* Try to include the file NAME.  Returns true if it does not
   exist. */
static int
try_include_file (const char *fname, int lnr, const char *name,
                  void (*outf)(char*))
{
  int rc;
  char *incfname;
  int repl_flag;

  repl_flag = !!strchr (name, '&');
  incfname = mk_include_name (name, repl_flag? host_triplet : NULL);
  rc = access (incfname, R_OK);
  if (rc && repl_flag)
    {
      free (incfname);
      incfname = mk_include_name (name, host_os);
      rc = access (incfname, R_OK);
    }
  if (!rc)
    include_file (fname, lnr, name, outf);

  free (incfname);
  return rc;
}


static int
write_special (const char *fname, int lnr, const char *tag)
{
  if (!strcmp (tag, "version"))
    {
      putchar ('\"');
      fputs (hdr_version, stdout);
      putchar ('\"');
    }
  else if (!strcmp (tag, "version-number"))
    {
      fputs (hdr_version_number, stdout);
    }
  else if (!strcmp (tag, "define:gpgrt_off_t"))
    {
      if (!replacement_for_off_type)
        {
          fprintf (stderr, "%s:%d: replacement for off_t not defined\n",
                   fname, lnr);
          exit (1);
        }
      else
        {
          if (!strcmp (replacement_for_off_type, "int64_t")
              && !stdint_h_included && have_stdint_h)
            {
              fputs ("#include <stdint.h>\n\n", stdout);
              stdint_h_included = 1;
            }
          printf ("typedef %s gpgrt_off_t;\n", replacement_for_off_type);
        }
    }
  else if (!strcmp (tag, "define:gpgrt_ssize_t"))
    {
      if (have_w64_system)
        {
          if (!stdint_h_included && have_stdint_h)
            {
              fputs ("# include <stdint.h>\n", stdout);
              stdint_h_included = 1;
            }
          fputs ("typedef int64_t gpgrt_ssize_t;\n", stdout);
        }
      else if (have_w32_system)
        {
          fputs ("typedef long    gpgrt_ssize_t;\n", stdout);
        }
      else
        {
          fputs ("#include <sys/types.h>\n"
                 "typedef ssize_t gpgrt_ssize_t;\n", stdout);
        }
    }
  else if (!strcmp (tag, "api_ssize_t"))
    {
      if (have_w32_system)
        fputs ("gpgrt_ssize_t", stdout);
      else
        fputs ("ssize_t", stdout);
    }
  else if (!strcmp (tag, "include:err-sources"))
    {
      write_sources_or_codes (NULL);
      include_file (fname, lnr, "err-sources.h.in", write_sources_or_codes);
    }
  else if (!strcmp (tag, "include:err-codes"))
    {
      write_sources_or_codes (NULL);
      include_file (fname, lnr, "err-codes.h.in", write_sources_or_codes);
    }
  else if (!strcmp (tag, "include:errnos"))
    {
      include_file (fname, lnr, "errnos.in", write_errnos_in);
    }
  else if (!strcmp (tag, "include:os-add"))
    {
      if (!strcmp (host_os, "mingw32"))
        {
          include_file (fname, lnr, "w32-add.h", write_line);
        }
      else if (!strcmp (host_os, "mingw32ce"))
        {
          include_file (fname, lnr, "w32-add.h", write_line);
          include_file (fname, lnr, "w32ce-add.h", write_line);
        }
    }
  else if (!strcmp (tag, "include:lock-obj"))
    {
      if (try_include_file (fname, lnr, "./lock-obj-pub.native.h", write_line))
        include_file (fname, lnr, "syscfg/lock-obj-pub.&.h", write_line);
    }
  else
    return 0; /* Unknown tag.  */

  return 1; /* Tag processed.  */
}


int
main (int argc, char **argv)
{
  FILE *fp;
  char line[LINESIZE];
  int lnr = 0;
  const char *fname, *s;
  char *p1, *p2;
  const char *config_h;

  if (argc)
    {
      argc--; argv++;
    }

  if (argc != 6)
    {
      fputs ("usage: " PGM
             " host_os host_triplet template.h config.h"
             " version version_number\n",
             stderr);
      return 1;
    }
  host_os = argv[0];
  host_triplet = argv[1];
  fname = argv[2];
  config_h = argv[3];
  hdr_version = argv[4];
  hdr_version_number = argv[5];

  srcdir = malloc (strlen (fname) + 2 + 1);
  if (!srcdir)
    {
      fputs (PGM ": out of core\n", stderr);
      return 1;
    }
  strcpy (srcdir, fname);
  p1 = strrchr (srcdir, '/');
  if (p1)
    p1[1] = 0;
  else
    strcpy (srcdir, "./");

  if (parse_config_h (config_h))
    return 1;

  fp = fopen (fname, "r");
  if (!fp)
    {
      fprintf (stderr, "%s:%d: can't open file: %s",
               fname, lnr, strerror (errno));
      return 1;
    }

  while (fgets (line, LINESIZE, fp))
    {
      size_t n = strlen (line);

      lnr++;
      if (!n || line[n-1] != '\n')
        {
          fprintf (stderr,
                   "%s:%d: trailing linefeed missing, line too long or "
                   "embedded nul character\n", fname, lnr);
          break;
        }
      line[--n] = 0;

      p1 = strchr (line, '@');
      p2 = p1? strchr (p1+1, '@') : NULL;
      if (!p1 || !p2 || p2-p1 == 1)
        {
          puts (line);
          continue;
        }
      *p1++ = 0;
      *p2++ = 0;
      fputs (line, stdout);

      if (!strcmp (p1, "configure_input"))
        {
          s = strrchr (fname, '/');
          printf ("Do not edit.  Generated from %s for %s.",
                  s? s+1 : fname, host_triplet);
          fputs (p2, stdout);
        }
      else if (!write_special (fname, lnr, p1))
        {
          putchar ('@');
          fputs (p1, stdout);
          putchar ('@');
          fputs (p2, stdout);
        }
      else if (p2 && *p2)
        {
          fputs (p2, stdout);
        }
      putchar ('\n');
    }

  if (ferror (fp))
    {
      fprintf (stderr, "%s:%d: error reading file: %s\n",
               fname, lnr, strerror (errno));
      return 1;
    }

  fputs ("/*\n"
         "Loc" "al Variables:\n"
         "buffer-read-only: t\n"
         "End:\n"
         "*/\n", stdout);

  if (ferror (stdout))
    {
      fprintf (stderr, PGM ": error writing to stdout: %s\n", strerror (errno));
      return 1;
    }

  fclose (fp);

  return 0;
}
