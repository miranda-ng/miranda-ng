/* ath.c - A Thread-safeness library.
 *  Copyright (C) 2002, 2003, 2004, 2011 Free Software Foundation, Inc.
 *  Copyright (C) 2014 g10 Code GmbH
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#if USE_POSIX_THREADS
# include <pthread.h>
#endif

#include "ath.h"

#if USE_POSIX_THREADS_WEAK
# if  !USE_POSIX_THREADS
#  error USE_POSIX_THREADS_WEAK but no USE_POSIX_THREADS
# endif
#endif


/* On an ELF system it is easy to use pthreads using weak references.
   Take care not to test the address of a weak referenced function we
   actually use; some GCC versions have a bug were &foo != NULL is
   always evaluated to true in PIC mode.  USING_PTHREAD_AS_DEFAULT is
   used by ath_install to detect the default usage of pthread.  */
#if USE_POSIX_THREADS_WEAK
# pragma weak pthread_cancel
# pragma weak pthread_mutex_init
# pragma weak pthread_mutex_lock
# pragma weak pthread_mutex_unlock
# pragma weak pthread_mutex_destroy
#endif

/* For the dummy interface.  The MUTEX_NOTINIT value is used to check
   that a mutex has been initialized.  */
#define MUTEX_NOTINIT	((ath_mutex_t) 0)
#define MUTEX_UNLOCKED	((ath_mutex_t) 1)
#define MUTEX_LOCKED	((ath_mutex_t) 2)
#define MUTEX_DESTROYED	((ath_mutex_t) 3)


/* Return the thread type from the option field. */
#define GET_OPTION(a)    ((a) & 0xff)



enum ath_thread_model {
  ath_model_undefined = 0,
  ath_model_none,          /* No thread support.  */
  ath_model_pthreads_weak, /* POSIX threads using weak symbols.  */
  ath_model_pthreads,      /* POSIX threads directly linked.  */
  ath_model_w32            /* Microsoft Windows threads.  */
};


/* The thread model in use.  */
static enum ath_thread_model thread_model;


/* Initialize the ath subsystem.  This is called as part of the
   Libgcrypt initialization.  It's purpose is to initialize the
   locking system.  It returns 0 on sucess or an ERRNO value on error.
   In the latter case it is not defined whether ERRNO was changed.

   Note: This should be called as early as possible because it is not
   always possible to detect the thread model to use while already
   running multi threaded.  */
int
ath_init (void)
{
  int err = 0;

  if (thread_model)
    return 0; /* Already initialized - no error.  */

  if (0)
    ;
#if USE_POSIX_THREADS_WEAK
  else if (pthread_cancel)
    {
      thread_model = ath_model_pthreads_weak;
    }
#endif
  else
    {
#if HAVE_W32_SYSTEM
      thread_model = ath_model_w32;
#elif USE_POSIX_THREADS && !USE_POSIX_THREADS_WEAK
      thread_model = ath_model_pthreads;
#else /*!USE_POSIX_THREADS*/
      /* Assume a single threaded application.  */
      thread_model = ath_model_none;
#endif /*!USE_POSIX_THREADS*/
    }

  return err;
}


/* Return the used thread model as string for display purposes an if
   R_MODEL is not null store its internal number at R_MODEL.  */
const char *
ath_get_model (int *r_model)
{
  if (r_model)
    *r_model = thread_model;
  switch (thread_model)
    {
    case ath_model_undefined:     return "undefined";
    case ath_model_none:          return "none";
    case ath_model_pthreads_weak: return "pthread(weak)";
    case ath_model_pthreads:      return "pthread";
    case ath_model_w32:           return "w32";
    default:                      return "?";
    }
}


/* This function was used in old Libgcrypt versions (via
   GCRYCTL_SET_THREAD_CBS) to register the thread callback functions.
   It is not anymore required.  However to allow existing code to
   continue to work, we keep this function and check that no user
   defined callbacks are used and that the requested thread system
   matches the one Libgcrypt is using.  */
gpg_err_code_t
ath_install (struct ath_ops *ath_ops)
{
  gpg_err_code_t rc;
  unsigned int thread_option;

  /* Fist call ath_init so that we know our thread model.  */
  rc = ath_init ();
  if (rc)
    return rc;

  /* Check if the requested thread option is compatible to the
     thread option we are already committed to.  */
  thread_option = ath_ops? GET_OPTION (ath_ops->option) : 0;

  /* Return an error if the requested thread model does not match the
     configured one.  */
  if (0)
    ;
#if USE_POSIX_THREADS
  else if (thread_model == ath_model_pthreads
           || thread_model == ath_model_pthreads_weak)
    {
      if (thread_option == ATH_THREAD_OPTION_PTHREAD)
        return 0; /* Okay - compatible.  */
      if (thread_option == ATH_THREAD_OPTION_PTH)
        return 0; /* Okay - compatible.  */
    }
#endif /*USE_POSIX_THREADS*/
  else if (thread_option == ATH_THREAD_OPTION_PTH)
    {
      if (thread_model == ath_model_none)
        return 0; /* Okay - compatible.  */
    }
  else if (thread_option == ATH_THREAD_OPTION_DEFAULT)
    return 0; /* No thread support requested.  */
#if HAVE_W32_SYSTEM
  else
    return 0; /* It is always enabled.  */
#endif /*HAVE_W32_SYSTEM*/

  return GPG_ERR_NOT_SUPPORTED;
}


/* Initialize a new mutex.  This function returns 0 on success or an
   system error code (i.e. an ERRNO value).  ERRNO may or may not be
   changed on error.  */
int
ath_mutex_init (ath_mutex_t *lock)
{
  int err;

  switch (thread_model)
    {
    case ath_model_none:
      *lock = MUTEX_UNLOCKED;
      err = 0;
      break;

#if USE_POSIX_THREADS
    case ath_model_pthreads:
    case ath_model_pthreads_weak:
      {
        pthread_mutex_t *plck;

        plck = malloc (sizeof *plck);
        if (!plck)
          err = errno? errno : ENOMEM;
        else
          {
            err = pthread_mutex_init (plck, NULL);
            if (err)
              free (plck);
            else
              *lock = (void*)plck;
          }
      }
      break;
#endif /*USE_POSIX_THREADS*/

#if HAVE_W32_SYSTEM
    case ath_model_w32:
      {
        CRITICAL_SECTION *csec;

        csec = malloc (sizeof *csec);
        if (!csec)
          err = errno? errno : ENOMEM;
        else
          {
            InitializeCriticalSection (csec);
            *lock = (void*)csec;
            err = 0;
          }
      }
      break;
#endif /*HAVE_W32_SYSTEM*/

    default:
      err = EINVAL;
      break;
    }

  return err;
}


/* Destroy a mutex.  This function is a NOP if LOCK is NULL.  If the
   mutex is still locked it can't be destroyed and the function
   returns EBUSY.  ERRNO may or may not be changed on error.  */
int
ath_mutex_destroy (ath_mutex_t *lock)
{
  int err;

  if (!*lock)
    return 0;

  switch (thread_model)
    {
    case ath_model_none:
      if (*lock != MUTEX_UNLOCKED)
        err = EBUSY;
      else
        {
          *lock = MUTEX_DESTROYED;
          err = 0;
        }
      break;

#if USE_POSIX_THREADS
    case ath_model_pthreads:
    case ath_model_pthreads_weak:
      {
        pthread_mutex_t *plck = (pthread_mutex_t*) (*lock);

        err = pthread_mutex_destroy (plck);
        if (!err)
          {
            free (plck);
            lock = NULL;
          }
      }
      break;
#endif /*USE_POSIX_THREADS*/

#if HAVE_W32_SYSTEM
    case ath_model_w32:
      {
        CRITICAL_SECTION *csec = (CRITICAL_SECTION *)(*lock);

        DeleteCriticalSection (csec);
        free (csec);
        err = 0;
      }
      break;
#endif /*HAVE_W32_SYSTEM*/

    default:
      err = EINVAL;
      break;
    }

  return err;
}


/* Lock the mutex LOCK.  On success the function returns 0; on error
   an error code.  ERRNO may or may not be changed on error.  */
int
ath_mutex_lock (ath_mutex_t *lock)
{
  int err;

  switch (thread_model)
    {
    case ath_model_none:
      if (*lock == MUTEX_NOTINIT)
	err = EINVAL;
      else if (*lock == MUTEX_UNLOCKED)
        {
          *lock = MUTEX_LOCKED;
          err = 0;
        }
      else
        err = EDEADLK;
      break;

#if USE_POSIX_THREADS
    case ath_model_pthreads:
    case ath_model_pthreads_weak:
      err = pthread_mutex_lock ((pthread_mutex_t*)(*lock));
      break;
#endif /*USE_POSIX_THREADS*/

#if HAVE_W32_SYSTEM
    case ath_model_w32:
      {
        CRITICAL_SECTION *csec = (CRITICAL_SECTION *)(*lock);

        EnterCriticalSection (csec);
        err = 0;
      }
      break;
#endif /*HAVE_W32_SYSTEM*/

    default:
      err = EINVAL;
      break;
    }

  return err;
}

/* Unlock the mutex LOCK.  On success the function returns 0; on error
   an error code.  ERRNO may or may not be changed on error.  */
int
ath_mutex_unlock (ath_mutex_t *lock)
{
  int err;

  switch (thread_model)
    {
    case ath_model_none:
      if (*lock == MUTEX_NOTINIT)
	err = EINVAL;
      else if (*lock == MUTEX_LOCKED)
        {
          *lock = MUTEX_UNLOCKED;
          err = 0;
        }
      else
        err = EPERM;
      break;

#if USE_POSIX_THREADS
    case ath_model_pthreads:
    case ath_model_pthreads_weak:
      err = pthread_mutex_unlock ((pthread_mutex_t*)(*lock));
      break;
#endif /*USE_POSIX_THREADS*/

#if HAVE_W32_SYSTEM
    case ath_model_w32:
      {
        CRITICAL_SECTION *csec = (CRITICAL_SECTION *)(*lock);

        LeaveCriticalSection (csec);
        err = 0;
      }
      break;
#endif /*HAVE_W32_SYSTEM*/

    default:
      err = EINVAL;
      break;
    }

  return err;
}
