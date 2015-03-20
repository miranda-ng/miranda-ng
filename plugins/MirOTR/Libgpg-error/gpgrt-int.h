/* gpgrt-int.h - Internal definitions
 * Copyright (C) 2014 g10 Code GmbH
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

#ifndef _GPGRT_GPGRT_INT_H
#define _GPGRT_GPGRT_INT_H

#include "gpg-error.h"
#include "visibility.h"

/* Local error function prototypes.  */
const char *_gpg_strerror (gpg_error_t err);
int _gpg_strerror_r (gpg_error_t err, char *buf, size_t buflen);
const char *_gpg_strsource (gpg_error_t err);
gpg_err_code_t _gpg_err_code_from_errno (int err);
int _gpg_err_code_to_errno (gpg_err_code_t code);
gpg_err_code_t _gpg_err_code_from_syserror (void);
void _gpg_err_set_errno (int err);

gpg_error_t _gpg_err_init (void);
void _gpg_err_deinit (int mode);
void _gpgrt_set_alloc_func (void *(*f)(void *a, size_t n));

void *_gpgrt_realloc (void *a, size_t n);
void *_gpgrt_malloc (size_t n);
void _gpgrt_free (void *a);

const char *_gpg_error_check_version (const char *req_version);

gpg_err_code_t _gpgrt_lock_init (gpgrt_lock_t *lockhd);
gpg_err_code_t _gpgrt_lock_lock (gpgrt_lock_t *lockhd);
gpg_err_code_t _gpgrt_lock_trylock (gpgrt_lock_t *lockhd);
gpg_err_code_t _gpgrt_lock_unlock (gpgrt_lock_t *lockhd);
gpg_err_code_t _gpgrt_lock_destroy (gpgrt_lock_t *lockhd);
gpg_err_code_t _gpgrt_yield (void);


/* Local prototypes for estream.  */
int _gpgrt_es_init (void);
void _gpgrt_set_syscall_clamp (void (*pre)(void), void (*post)(void));

gpgrt_stream_t _gpgrt_fopen (const char *_GPGRT__RESTRICT path,
                             const char *_GPGRT__RESTRICT mode);
gpgrt_stream_t _gpgrt_mopen (void *_GPGRT__RESTRICT data,
                             size_t data_n, size_t data_len,
                             unsigned int grow,
                             void *(*func_realloc) (void *mem, size_t size),
                             void (*func_free) (void *mem),
                             const char *_GPGRT__RESTRICT mode);
gpgrt_stream_t _gpgrt_fopenmem (size_t memlimit,
                                const char *_GPGRT__RESTRICT mode);
gpgrt_stream_t _gpgrt_fopenmem_init (size_t memlimit,
                                     const char *_GPGRT__RESTRICT mode,
                                     const void *data, size_t datalen);
gpgrt_stream_t _gpgrt_fdopen    (int filedes, const char *mode);
gpgrt_stream_t _gpgrt_fdopen_nc (int filedes, const char *mode);
gpgrt_stream_t _gpgrt_sysopen    (gpgrt_syshd_t *syshd, const char *mode);
gpgrt_stream_t _gpgrt_sysopen_nc (gpgrt_syshd_t *syshd, const char *mode);
gpgrt_stream_t _gpgrt_fpopen    (FILE *fp, const char *mode);
gpgrt_stream_t _gpgrt_fpopen_nc (FILE *fp, const char *mode);
gpgrt_stream_t _gpgrt_freopen (const char *_GPGRT__RESTRICT path,
                               const char *_GPGRT__RESTRICT mode,
                               gpgrt_stream_t _GPGRT__RESTRICT stream);
gpgrt_stream_t _gpgrt_fopencookie (void *_GPGRT__RESTRICT cookie,
                                   const char *_GPGRT__RESTRICT mode,
                                   gpgrt_cookie_io_functions_t functions);
int _gpgrt_fclose (gpgrt_stream_t stream);
int _gpgrt_fclose_snatch (gpgrt_stream_t stream,
                          void **r_buffer, size_t *r_buflen);
int _gpgrt_onclose (gpgrt_stream_t stream, int mode,
                    void (*fnc) (gpgrt_stream_t, void*), void *fnc_value);
int _gpgrt_fileno (gpgrt_stream_t stream);
int _gpgrt_fileno_unlocked (gpgrt_stream_t stream);
int _gpgrt_syshd (gpgrt_stream_t stream, gpgrt_syshd_t *syshd);
int _gpgrt_syshd_unlocked (gpgrt_stream_t stream, gpgrt_syshd_t *syshd);

void _gpgrt__set_std_fd (int no, int fd);
gpgrt_stream_t _gpgrt__get_std_stream (int fd);

void _gpgrt_flockfile (gpgrt_stream_t stream);
int  _gpgrt_ftrylockfile (gpgrt_stream_t stream);
void _gpgrt_funlockfile (gpgrt_stream_t stream);

int _gpgrt_feof (gpgrt_stream_t stream);
int _gpgrt_feof_unlocked (gpgrt_stream_t stream);
int _gpgrt_ferror (gpgrt_stream_t stream);
int _gpgrt_ferror_unlocked (gpgrt_stream_t stream);
void _gpgrt_clearerr (gpgrt_stream_t stream);
void _gpgrt_clearerr_unlocked (gpgrt_stream_t stream);
int _gpgrt__pending (gpgrt_stream_t stream);
int _gpgrt__pending_unlocked (gpgrt_stream_t stream);

int _gpgrt_fflush (gpgrt_stream_t stream);
int _gpgrt_fseek (gpgrt_stream_t stream, long int offset, int whence);
int _gpgrt_fseeko (gpgrt_stream_t stream, gpgrt_off_t offset, int whence);
long int _gpgrt_ftell (gpgrt_stream_t stream);
gpgrt_off_t _gpgrt_ftello (gpgrt_stream_t stream);
void _gpgrt_rewind (gpgrt_stream_t stream);

int _gpgrt_fgetc (gpgrt_stream_t stream);
int _gpgrt_fputc (int c, gpgrt_stream_t stream);

int _gpgrt__getc_underflow (gpgrt_stream_t stream);
int _gpgrt__putc_overflow (int c, gpgrt_stream_t stream);

/* Note: Keeps the next two macros in sync
         with their counterparts in gpg-error.h. */
#define _gpgrt_getc_unlocked(stream)				\
  (((!(stream)->flags.writing)					\
    && ((stream)->data_offset < (stream)->data_len)		\
    && (! (stream)->unread_data_len))				\
  ? ((int) (stream)->buffer[((stream)->data_offset)++])		\
  : _gpgrt__getc_underflow ((stream)))

#define _gpgrt_putc_unlocked(c, stream)				\
  (((stream)->flags.writing					\
    && ((stream)->data_offset < (stream)->buffer_size)		\
    && (c != '\n'))						\
  ? ((int) ((stream)->buffer[((stream)->data_offset)++] = (c)))	\
  : _gpgrt__putc_overflow ((c), (stream)))

int _gpgrt_ungetc (int c, gpgrt_stream_t stream);

int _gpgrt_read (gpgrt_stream_t _GPGRT__RESTRICT stream,
                 void *_GPGRT__RESTRICT buffer, size_t bytes_to_read,
                 size_t *_GPGRT__RESTRICT bytes_read);
int _gpgrt_write (gpgrt_stream_t _GPGRT__RESTRICT stream,
                  const void *_GPGRT__RESTRICT buffer, size_t bytes_to_write,
                  size_t *_GPGRT__RESTRICT bytes_written);
int _gpgrt_write_sanitized (gpgrt_stream_t _GPGRT__RESTRICT stream,
                            const void *_GPGRT__RESTRICT buffer, size_t length,
                            const char *delimiters,
                            size_t *_GPGRT__RESTRICT bytes_written);
int _gpgrt_write_hexstring (gpgrt_stream_t _GPGRT__RESTRICT stream,
                            const void *_GPGRT__RESTRICT buffer, size_t length,
                            int reserved,
                            size_t *_GPGRT__RESTRICT bytes_written);

size_t _gpgrt_fread (void *_GPGRT__RESTRICT ptr, size_t size, size_t nitems,
                     gpgrt_stream_t _GPGRT__RESTRICT stream);
size_t _gpgrt_fwrite (const void *_GPGRT__RESTRICT ptr,
                      size_t size, size_t memb,
                      gpgrt_stream_t _GPGRT__RESTRICT stream);

char *_gpgrt_fgets (char *_GPGRT__RESTRICT s, int n,
                    gpgrt_stream_t _GPGRT__RESTRICT stream);
int _gpgrt_fputs (const char *_GPGRT__RESTRICT s,
                  gpgrt_stream_t _GPGRT__RESTRICT stream);
int _gpgrt_fputs_unlocked (const char *_GPGRT__RESTRICT s,
                           gpgrt_stream_t _GPGRT__RESTRICT stream);

gpgrt_ssize_t _gpgrt_getline (char *_GPGRT__RESTRICT *_GPGRT__RESTRICT lineptr,
                              size_t *_GPGRT__RESTRICT n,
                              gpgrt_stream_t stream);
gpgrt_ssize_t _gpgrt_read_line (gpgrt_stream_t stream,
                                char **addr_of_buffer, size_t *length_of_buffer,
                                size_t *max_length);

int _gpgrt_fprintf (gpgrt_stream_t _GPGRT__RESTRICT stream,
                    const char *_GPGRT__RESTRICT format, ...)
                    _GPGRT_GCC_A_PRINTF(2,3);
int _gpgrt_fprintf_unlocked (gpgrt_stream_t _GPGRT__RESTRICT stream,
                             const char *_GPGRT__RESTRICT format, ...)
                             _GPGRT_GCC_A_PRINTF(2,3);

int _gpgrt_vfprintf (gpgrt_stream_t _GPGRT__RESTRICT stream,
                     const char *_GPGRT__RESTRICT format, va_list ap)
                     _GPGRT_GCC_A_PRINTF(2,0);
int _gpgrt_vfprintf_unlocked (gpgrt_stream_t _GPGRT__RESTRICT stream,
                              const char *_GPGRT__RESTRICT format, va_list ap)
                              _GPGRT_GCC_A_PRINTF(2,0);

int _gpgrt_setvbuf (gpgrt_stream_t _GPGRT__RESTRICT stream,
                    char *_GPGRT__RESTRICT buf, int mode, size_t size);

void _gpgrt_set_binary (gpgrt_stream_t stream);

gpgrt_stream_t _gpgrt_tmpfile (void);

void _gpgrt_opaque_set (gpgrt_stream_t _GPGRT__RESTRICT stream,
                        void *_GPGRT__RESTRICT opaque);
void *_gpgrt_opaque_get (gpgrt_stream_t stream);

void _gpgrt_fname_set (gpgrt_stream_t stream, const char *fname);
const char *_gpgrt_fname_get (gpgrt_stream_t stream);

#include "estream-printf.h"


#endif /*_GPGRT_GPGRT_INT_H*/
