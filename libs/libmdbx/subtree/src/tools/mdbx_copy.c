/* mdbx_copy.c - memory-mapped database backup tool */

/*
 * Copyright 2015-2019 Leonid Yuriev <leo@yuriev.ru>
 * and other libmdbx authors: please see AUTHORS file.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted only as authorized by the OpenLDAP
 * Public License.
 *
 * A copy of this license is available in the file LICENSE in the
 * top-level directory of the distribution or, alternatively, at
 * <http://www.OpenLDAP.org/license.html>. */

#ifdef _MSC_VER
#if _MSC_VER > 1800
#pragma warning(disable : 4464) /* relative include path contains '..' */
#endif
#pragma warning(disable : 4996) /* The POSIX name is deprecated... */
#endif                          /* _MSC_VER (warnings) */

#include "../bits.h"

#if defined(_WIN32) || defined(_WIN64)
#include "wingetopt.h"

static volatile BOOL user_break;
static BOOL WINAPI ConsoleBreakHandlerRoutine(DWORD dwCtrlType) {
  (void)dwCtrlType;
  user_break = true;
  return true;
}

#else /* WINDOWS */

static volatile sig_atomic_t user_break;
static void signal_handler(int sig) {
  (void)sig;
  user_break = 1;
}

#endif /* !WINDOWS */

int main(int argc, char *argv[]) {
  int rc;
  MDBX_env *env = NULL;
  const char *progname = argv[0], *act;
  unsigned flags = MDBX_RDONLY;
  unsigned cpflags = 0;

  for (; argc > 1 && argv[1][0] == '-'; argc--, argv++) {
    if (argv[1][1] == 'n' && argv[1][2] == '\0')
      flags |= MDBX_NOSUBDIR;
    else if (argv[1][1] == 'c' && argv[1][2] == '\0')
      cpflags |= MDBX_CP_COMPACT;
    else if (argv[1][1] == 'V' && argv[1][2] == '\0') {
      printf("%s (%s, build %s)\n", mdbx_version.git.describe,
             mdbx_version.git.datetime, mdbx_build.datetime);
      exit(EXIT_SUCCESS);
    } else
      argc = 0;
  }

  if (argc < 2 || argc > 3) {
    fprintf(stderr, "usage: %s [-V] [-c] [-n] srcpath [dstpath]\n", progname);
    exit(EXIT_FAILURE);
  }

#if defined(_WIN32) || defined(_WIN64)
  SetConsoleCtrlHandler(ConsoleBreakHandlerRoutine, true);
#else
#ifdef SIGPIPE
  signal(SIGPIPE, signal_handler);
#endif
#ifdef SIGHUP
  signal(SIGHUP, signal_handler);
#endif
  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);
#endif /* !WINDOWS */

  act = "opening environment";
  rc = mdbx_env_create(&env);
  if (rc == MDBX_SUCCESS) {
    rc = mdbx_env_open(env, argv[1], flags, 0640);
  }
  if (rc == MDBX_SUCCESS) {
    act = "copying";
    if (argc == 2) {
      mdbx_filehandle_t fd;
#if defined(_WIN32) || defined(_WIN64)
      fd = GetStdHandle(STD_OUTPUT_HANDLE);
#else
      fd = fileno(stdout);
#endif
      rc = mdbx_env_copy2fd(env, fd, cpflags);
    } else
      rc = mdbx_env_copy(env, argv[2], cpflags);
  }
  if (rc)
    fprintf(stderr, "%s: %s failed, error %d (%s)\n", progname, act, rc,
            mdbx_strerror(rc));
  mdbx_env_close(env);

  return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}
