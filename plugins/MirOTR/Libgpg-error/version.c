/* version.c - Version checking
 * Copyright (C) 2001, 2002, 2012, 2013, 2014 g10 Code GmbH
 *
 * This file is part of libgpg-error.
 *
 * libgpg-error is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * libgpg-error is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <gpg-error.h>


#define digitp(a) ((a) >= '0' && (a) <= '9')


/* This is actually a dummy function to make sure that is module is
   not empty.  Some compilers barf on empty modules.  */
static const char *
cright_blurb (void)
{
  static const char blurb[] =
    "\n\n"
    "This is Libgpg-error " PACKAGE_VERSION " - An error code library\n"
    "Copyright 2003, 2004, 2010, 2013, 2014, 2015 g10 Code GmbH\n"
    "\n"
    "(" BUILD_REVISION " " BUILD_TIMESTAMP ")\n"
    "\n\n";
  return blurb;
}


static const char*
parse_version_number (const char *s, int *number)
{
  int val = 0;

  if (*s == '0' && digitp (s[1]))
    return NULL;  /* Leading zeros are not allowed.  */
  for (; digitp (*s); s++)
    {
      val *= 10;
      val += *s - '0';
    }
  *number = val;
  return val < 0 ? NULL : s;
}


static const char *
parse_version_string (const char *s, int *major, int *minor)
{
  s = parse_version_number (s, major);
  if (!s || *s != '.')
    return NULL;
  s++;
  s = parse_version_number (s, minor);
  if (!s)
    return NULL;
  return s;  /* Patchlevel.  */
}


static const char *
compare_versions (const char *my_version, const char *req_version)
{
  int my_major, my_minor;
  int rq_major, rq_minor;
  const char *my_plvl, *rq_plvl;

  if (!req_version)
    return my_version;
  if (!my_version)
    return NULL;

  my_plvl = parse_version_string (my_version, &my_major, &my_minor);
  if (!my_plvl)
    return NULL;	/* Very strange: our own version is bogus.  */
  rq_plvl = parse_version_string(req_version, &rq_major, &rq_minor);
  if (!rq_plvl)
    return NULL;	/* Requested version string is invalid.  */

  if (my_major > rq_major
      || (my_major == rq_major && my_minor >= rq_minor))
    {
      return my_version;
    }
  return NULL;
}


/*
 * Check that the the version of the library is at minimum REQ_VERSION
 * and return the actual version string; return NULL if the condition
 * is not met.  If NULL is passed to this function, no check is done
 * and the version string is simply returned.
 */
const char *
_gpg_error_check_version (const char *req_version)
{
  if (req_version && req_version[0] == 1 && req_version[1] == 1)
    return cright_blurb ();
  return compare_versions (PACKAGE_VERSION, req_version);
}
