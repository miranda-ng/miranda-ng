/* ath.h - Thread-safeness library.
 * Copyright (C) 2002, 2003, 2004, 2011 Free Software Foundation, Inc.
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser general Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ATH_H
#define ATH_H

#include <config.h>

#ifdef _WIN32
# include <winsock2.h>
# include <windows.h>
#else /* !_WIN32 */
# ifdef HAVE_SYS_SELECT_H
#  include <sys/select.h>
# else
#  include <sys/time.h>
# endif
# include <sys/types.h>
# ifdef HAVE_SYS_MSG_H
#  include <sys/msg.h>  /* (e.g. for zOS) */
# endif
# include <sys/socket.h>
#endif /* !_WIN32 */
#include <gpg-error.h>



/* Define _ATH_EXT_SYM_PREFIX if you want to give all external symbols
   a prefix.  */
#define _ATH_EXT_SYM_PREFIX _gcry_

#ifdef _ATH_EXT_SYM_PREFIX
#define _ATH_PREFIX1(x,y) x ## y
#define _ATH_PREFIX2(x,y) _ATH_PREFIX1(x,y)
#define _ATH_PREFIX(x) _ATH_PREFIX2(_ATH_EXT_SYM_PREFIX,x)
#define ath_install _ATH_PREFIX(ath_install)
#define ath_init _ATH_PREFIX(ath_init)
#define ath_get_model _ATH_PREFIX(ath_get_model)
#define ath_mutex_init _ATH_PREFIX(ath_mutex_init)
#define ath_mutex_destroy _ATH_PREFIX(ath_mutex_destroy)
#define ath_mutex_lock _ATH_PREFIX(ath_mutex_lock)
#define ath_mutex_unlock _ATH_PREFIX(ath_mutex_unlock)
#endif


enum ath_thread_option
  {
    ATH_THREAD_OPTION_DEFAULT = 0,
    ATH_THREAD_OPTION_USER = 1,
    ATH_THREAD_OPTION_PTH = 2,
    ATH_THREAD_OPTION_PTHREAD = 3
  };

struct ath_ops
{
  /* The OPTION field encodes the thread model and the version number
     of this structure.
       Bits  7 - 0  are used for the thread model
       Bits 15 - 8  are used for the version number.
  */
  unsigned int option;

};

gpg_err_code_t ath_install (struct ath_ops *ath_ops);
int ath_init (void);
const char *ath_get_model (int *r_model);

/* Functions for mutual exclusion.  */
typedef void *ath_mutex_t;

int ath_mutex_init (ath_mutex_t *mutex);
int ath_mutex_destroy (ath_mutex_t *mutex);
int ath_mutex_lock (ath_mutex_t *mutex);
int ath_mutex_unlock (ath_mutex_t *mutex);

#endif	/* ATH_H */
