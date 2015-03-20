/* visibility.c - Wrapper for all public functions.
 * Copyright (C) 2014  g10 Code GmbH
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

#include <config.h>
#include <stdarg.h>

#define _GPGRT_INCL_BY_VISIBILITY_C 1
#include "gpgrt-int.h"

const char *
gpg_strerror (gpg_error_t err)
{
  return _gpg_strerror (err);
}

int
gpg_strerror_r (gpg_error_t err, char *buf, size_t buflen)
{
  return _gpg_strerror_r (err, buf, buflen);
}

const char *
gpg_strsource (gpg_error_t err)
{
  return _gpg_strsource (err);
}

gpg_err_code_t
gpg_err_code_from_errno (int err)
{
  return _gpg_err_code_from_errno (err);
}

int
gpg_err_code_to_errno (gpg_err_code_t code)
{
  return _gpg_err_code_to_errno (code);
}

gpg_err_code_t
gpg_err_code_from_syserror (void)
{
  return _gpg_err_code_from_syserror ();
}

void
gpg_err_set_errno (int err)
{
  _gpg_err_set_errno (err);
}


gpg_error_t
gpg_err_init (void)
{
  return _gpg_err_init ();
}

void
gpg_err_deinit (int mode)
{
  _gpg_err_deinit (mode);
}

const char *
gpg_error_check_version (const char *req_version)
{
  return _gpg_error_check_version (req_version);
}

const char *
gpgrt_check_version (const char *req_version)
{
  return _gpg_error_check_version (req_version);
}

void
gpgrt_set_syscall_clamp (void (*pre)(void), void (*post)(void))
{
  _gpgrt_set_syscall_clamp (pre, post);
}

void
gpgrt_set_alloc_func (void *(*f)(void *a, size_t n))
{
  _gpgrt_set_alloc_func (f);
}


gpg_err_code_t
gpgrt_lock_init (gpgrt_lock_t *lockhd)
{
  return _gpgrt_lock_init (lockhd);
}

gpg_err_code_t
gpgrt_lock_lock (gpgrt_lock_t *lockhd)
{
  return _gpgrt_lock_lock (lockhd);
}

gpg_err_code_t
gpgrt_lock_trylock (gpgrt_lock_t *lockhd)
{
  return _gpgrt_lock_trylock (lockhd);
}

gpg_err_code_t
gpgrt_lock_unlock (gpgrt_lock_t *lockhd)
{
  return _gpgrt_lock_unlock (lockhd);
}

gpg_err_code_t
gpgrt_lock_destroy (gpgrt_lock_t *lockhd)
{
  return _gpgrt_lock_destroy (lockhd);
}

gpg_err_code_t
gpgrt_yield (void)
{
  return _gpgrt_yield ();
}



estream_t
gpgrt_fopen (const char *_GPGRT__RESTRICT path,
             const char *_GPGRT__RESTRICT mode)
{
  return _gpgrt_fopen (path, mode);
}

estream_t
gpgrt_mopen (void *_GPGRT__RESTRICT data, size_t data_n, size_t data_len,
             unsigned int grow,
             void *(*func_realloc) (void *mem, size_t size),
             void (*func_free) (void *mem),
             const char *_GPGRT__RESTRICT mode)
{
  return _gpgrt_mopen (data, data_n, data_len, grow, func_realloc, func_free,
                       mode);
}

estream_t
gpgrt_fopenmem (size_t memlimit, const char *_GPGRT__RESTRICT mode)
{
  return _gpgrt_fopenmem (memlimit, mode);
}

estream_t
gpgrt_fopenmem_init (size_t memlimit, const char *_GPGRT__RESTRICT mode,
                     const void *data, size_t datalen)
{
  return _gpgrt_fopenmem_init (memlimit, mode, data, datalen);
}

estream_t
gpgrt_fdopen (int filedes, const char *mode)
{
  return _gpgrt_fdopen (filedes, mode);
}

estream_t
gpgrt_fdopen_nc (int filedes, const char *mode)
{
  return _gpgrt_fdopen_nc (filedes, mode);
}

estream_t
gpgrt_sysopen (es_syshd_t *syshd, const char *mode)
{
  return _gpgrt_sysopen (syshd, mode);
}

estream_t
gpgrt_sysopen_nc (es_syshd_t *syshd, const char *mode)
{
  return _gpgrt_sysopen_nc (syshd, mode);
}

estream_t
gpgrt_fpopen (FILE *fp, const char *mode)
{
  return _gpgrt_fpopen (fp, mode);
}

estream_t
gpgrt_fpopen_nc (FILE *fp, const char *mode)
{
  return _gpgrt_fpopen_nc (fp, mode);
}

estream_t
gpgrt_freopen (const char *_GPGRT__RESTRICT path,
               const char *_GPGRT__RESTRICT mode,
               estream_t _GPGRT__RESTRICT stream)
{
  return _gpgrt_freopen (path, mode, stream);
}

estream_t
gpgrt_fopencookie (void *_GPGRT__RESTRICT cookie,
                   const char *_GPGRT__RESTRICT mode,
                   gpgrt_cookie_io_functions_t functions)
{
  return _gpgrt_fopencookie (cookie, mode, functions);
}

int
gpgrt_fclose (estream_t stream)
{
  return _gpgrt_fclose (stream);
}

int
gpgrt_fclose_snatch (estream_t stream, void **r_buffer, size_t *r_buflen)
{
  return _gpgrt_fclose_snatch (stream, r_buffer, r_buflen);
}

int
gpgrt_onclose (estream_t stream, int mode,
               void (*fnc) (estream_t, void*), void *fnc_value)
{
  return _gpgrt_onclose (stream, mode, fnc, fnc_value);
}

int
gpgrt_fileno (estream_t stream)
{
  return _gpgrt_fileno (stream);
}

int
gpgrt_fileno_unlocked (estream_t stream)
{
  return _gpgrt_fileno_unlocked (stream);
}

int
gpgrt_syshd (estream_t stream, es_syshd_t *syshd)
{
  return _gpgrt_syshd (stream, syshd);
}

int
gpgrt_syshd_unlocked (estream_t stream, es_syshd_t *syshd)
{
  return _gpgrt_syshd_unlocked (stream, syshd);
}

void
_gpgrt_set_std_fd (int no, int fd)
{
  _gpgrt__set_std_fd (no, fd);  /* (double dash in name) */
}

estream_t
_gpgrt_get_std_stream (int fd)
{
  return _gpgrt__get_std_stream (fd);  /* (double dash in name) */
}

void
gpgrt_flockfile (estream_t stream)
{
  _gpgrt_flockfile (stream);
}

int
gpgrt_ftrylockfile (estream_t stream)
{
  return _gpgrt_ftrylockfile (stream);
}

void
gpgrt_funlockfile (estream_t stream)
{
  _gpgrt_funlockfile (stream);
}

int
_gpgrt_pending (estream_t stream)
{
  return _gpgrt__pending (stream);
}

int
_gpgrt_pending_unlocked (estream_t stream)
{
  return _gpgrt__pending_unlocked (stream);
}

int
gpgrt_feof (estream_t stream)
{
  return _gpgrt_feof (stream);
}

int
gpgrt_feof_unlocked (estream_t stream)
{
  return _gpgrt_feof_unlocked (stream);
}

int
gpgrt_ferror (estream_t stream)
{
  return _gpgrt_ferror (stream);
}

int
gpgrt_ferror_unlocked (estream_t stream)
{
  return _gpgrt_ferror_unlocked (stream);
}

void
gpgrt_clearerr (estream_t stream)
{
  _gpgrt_clearerr (stream);
}

void
gpgrt_clearerr_unlocked (estream_t stream)
{
  _gpgrt_clearerr_unlocked (stream);
}

int
gpgrt_fflush (estream_t stream)
{
  return _gpgrt_fflush (stream);
}

int
gpgrt_fseek (estream_t stream, long int offset, int whence)
{
  return _gpgrt_fseek (stream, offset, whence);
}

int
gpgrt_fseeko (estream_t stream, gpgrt_off_t offset, int whence)
{
  return _gpgrt_fseeko (stream, offset, whence);
}

long int
gpgrt_ftell (estream_t stream)
{
  return _gpgrt_ftell (stream);
}

gpgrt_off_t
gpgrt_ftello (estream_t stream)
{
  return _gpgrt_ftello (stream);
}

void
gpgrt_rewind (estream_t stream)
{
  _gpgrt_rewind (stream);
}

int
gpgrt_fgetc (estream_t stream)
{
  return _gpgrt_fgetc (stream);
}

int
_gpgrt_getc_underflow (estream_t stream)
{
  return _gpgrt__getc_underflow (stream);
}

int
gpgrt_fputc (int c, estream_t stream)
{
  return _gpgrt_fputc (c, stream);
}

int
_gpgrt_putc_overflow (int c, estream_t stream)
{
  return _gpgrt__putc_overflow (c, stream);
}

int
gpgrt_ungetc (int c, estream_t stream)
{
  return _gpgrt_ungetc (c, stream);
}

int
gpgrt_read (estream_t _GPGRT__RESTRICT stream,
            void *_GPGRT__RESTRICT buffer, size_t bytes_to_read,
            size_t *_GPGRT__RESTRICT bytes_read)
{
  return _gpgrt_read (stream, buffer, bytes_to_read, bytes_read);
}

int
gpgrt_write (estream_t _GPGRT__RESTRICT stream,
             const void *_GPGRT__RESTRICT buffer, size_t bytes_to_write,
             size_t *_GPGRT__RESTRICT bytes_written)
{
  return _gpgrt_write (stream, buffer, bytes_to_write, bytes_written);
}

int
gpgrt_write_sanitized (estream_t _GPGRT__RESTRICT stream,
                       const void * _GPGRT__RESTRICT buffer, size_t length,
                       const char * delimiters,
                       size_t * _GPGRT__RESTRICT bytes_written)
{
  return _gpgrt_write_sanitized (stream, buffer, length, delimiters,
                                 bytes_written);
}

int
gpgrt_write_hexstring (estream_t _GPGRT__RESTRICT stream,
                       const void *_GPGRT__RESTRICT buffer, size_t length,
                       int reserved, size_t *_GPGRT__RESTRICT bytes_written )
{
  return _gpgrt_write_hexstring (stream, buffer, length, reserved,
                                 bytes_written);
}

size_t
gpgrt_fread (void *_GPGRT__RESTRICT ptr, size_t size, size_t nitems,
             estream_t _GPGRT__RESTRICT stream)
{
  return _gpgrt_fread (ptr, size, nitems, stream);
}

size_t
gpgrt_fwrite (const void *_GPGRT__RESTRICT ptr, size_t size, size_t nitems,
              estream_t _GPGRT__RESTRICT stream)
{
  return _gpgrt_fwrite (ptr, size, nitems, stream);
}

char *
gpgrt_fgets (char *_GPGRT__RESTRICT buffer, int length,
             estream_t _GPGRT__RESTRICT stream)
{
  return _gpgrt_fgets (buffer, length, stream);
}

int
gpgrt_fputs (const char *_GPGRT__RESTRICT s, estream_t _GPGRT__RESTRICT stream)
{
  return _gpgrt_fputs (s, stream);
}

int
gpgrt_fputs_unlocked (const char *_GPGRT__RESTRICT s,
                      estream_t _GPGRT__RESTRICT stream)
{
  return _gpgrt_fputs_unlocked (s, stream);
}

gpgrt_ssize_t
gpgrt_getline (char *_GPGRT__RESTRICT *_GPGRT__RESTRICT lineptr,
               size_t *_GPGRT__RESTRICT n, estream_t _GPGRT__RESTRICT stream)
{
  return _gpgrt_getline (lineptr, n, stream);
}

gpgrt_ssize_t
gpgrt_read_line (estream_t stream,
                 char **addr_of_buffer, size_t *length_of_buffer,
                 size_t *max_length)
{
  return _gpgrt_read_line (stream, addr_of_buffer, length_of_buffer,
                           max_length);
}

void
gpgrt_free (void *a)
{
  if (a)
    _gpgrt_free (a);
}

int
gpgrt_vfprintf (estream_t _GPGRT__RESTRICT stream,
                const char *_GPGRT__RESTRICT format,
                va_list ap)
{
  return _gpgrt_vfprintf (stream, format, ap);
}

int
gpgrt_vfprintf_unlocked (estream_t _GPGRT__RESTRICT stream,
                          const char *_GPGRT__RESTRICT format,
                          va_list ap)
{
  return _gpgrt_vfprintf_unlocked (stream, format, ap);
}

int
gpgrt_printf (const char *_GPGRT__RESTRICT format, ...)
{
  va_list ap;
  int rc;

  va_start (ap, format);
  rc = _gpgrt_vfprintf (es_stdout, format, ap);
  va_end (ap);

  return rc;
}

int
gpgrt_printf_unlocked (const char *_GPGRT__RESTRICT format, ...)
{
  va_list ap;
  int rc;

  va_start (ap, format);
  rc = _gpgrt_vfprintf_unlocked (es_stdout, format, ap);
  va_end (ap);

  return rc;
}

int
gpgrt_fprintf (estream_t _GPGRT__RESTRICT stream,
               const char *_GPGRT__RESTRICT format, ...)
{
  va_list ap;
  int rc;

  va_start (ap, format);
  rc = _gpgrt_vfprintf (stream, format, ap);
  va_end (ap);

  return rc;
}

int
gpgrt_fprintf_unlocked (estream_t _GPGRT__RESTRICT stream,
                        const char *_GPGRT__RESTRICT format, ...)
{
  va_list ap;
  int rc;

  va_start (ap, format);
  rc = _gpgrt_vfprintf_unlocked (stream, format, ap);
  va_end (ap);

  return rc;
}

int
gpgrt_setvbuf (estream_t _GPGRT__RESTRICT stream,
                char *_GPGRT__RESTRICT buf, int type, size_t size)
{
  return _gpgrt_setvbuf (stream, buf, type, size);
}

void
gpgrt_setbuf (estream_t _GPGRT__RESTRICT stream, char *_GPGRT__RESTRICT buf)
{
  _gpgrt_setvbuf (stream, buf, buf? _IOFBF : _IONBF, BUFSIZ);
}

void
gpgrt_set_binary (estream_t stream)
{
  _gpgrt_set_binary (stream);
}

estream_t
gpgrt_tmpfile (void)
{
  return _gpgrt_tmpfile ();
}

void
gpgrt_opaque_set (estream_t stream, void *opaque)
{
  _gpgrt_opaque_set (stream, opaque);
}

void *
gpgrt_opaque_get (estream_t stream)
{
  return _gpgrt_opaque_get (stream);
}

void
gpgrt_fname_set (estream_t stream, const char *fname)
{
  _gpgrt_fname_set (stream, fname);
}

const char *
gpgrt_fname_get (estream_t stream)
{
  return _gpgrt_fname_get (stream);
}

int
gpgrt_asprintf (char **r_buf, const char *_GPGRT__RESTRICT format, ...)
{
  va_list ap;
  int rc;

  va_start (ap, format);
  rc = _gpgrt_estream_vasprintf (r_buf, format, ap);
  va_end (ap);

  return rc;
}

int
gpgrt_vasprintf (char **r_buf, const char *_GPGRT__RESTRICT format, va_list ap)
{
  return _gpgrt_estream_vasprintf (r_buf, format, ap);
}

char *
gpgrt_bsprintf (const char *_GPGRT__RESTRICT format, ...)
{
  int rc;
  va_list ap;
  char *buf;

  va_start (ap, format);
  rc = _gpgrt_estream_vasprintf (&buf, format, ap);
  va_end (ap);
  if (rc < 0)
    return NULL;
  return buf;
}

char *
gpgrt_vbsprintf (const char *_GPGRT__RESTRICT format, va_list ap)
{
  int rc;
  char *buf;

  rc = _gpgrt_estream_vasprintf (&buf, format, ap);
  if (rc < 0)
    return NULL;
  return buf;
}

int
gpgrt_snprintf (char *buf, size_t bufsize, const char *format, ...)
{
  int rc;
  va_list arg_ptr;

  va_start (arg_ptr, format);
  rc = _gpgrt_estream_vsnprintf (buf, bufsize, format, arg_ptr);
  va_end (arg_ptr);

  return rc;
}

int
gpgrt_vsnprintf (char *buf, size_t bufsize,
                 const char *format, va_list arg_ptr)
{
  return _gpgrt_estream_vsnprintf (buf, bufsize, format, arg_ptr);
}
