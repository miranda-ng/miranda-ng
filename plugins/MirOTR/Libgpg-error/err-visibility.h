/* err-visibility.h - Set visibility attribute
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

#ifndef _GPGRT_VISIBILITY_H
#define _GPGRT_VISIBILITY_H

/* Include the main header here so that public symbols are mapped to
   the internal underscored ones.  */
#ifdef _GPGRT_INCL_BY_VISIBILITY_C
# include "gpgrt-int.h"
#endif


/* Our use of the ELF visibility feature works by passing
   -fvisibiliy=hidden on the command line and by explicitly marking
   all exported functions as visible.

   NOTE: When adding new functions, please make sure to add them to
         gpg-error.vers and gpg-error.def.in as well.  */

#ifdef _GPGRT_INCL_BY_VISIBILITY_C

# ifdef GPGRT_USE_VISIBILITY
#  define MARK_VISIBLE(name) \
     extern __typeof__ (name) name __attribute__ ((visibility("default")));
# else
#  define MARK_VISIBLE(name) /* */
# endif

MARK_VISIBLE (gpg_strerror)
MARK_VISIBLE (gpg_strerror_r)
MARK_VISIBLE (gpg_strsource)
MARK_VISIBLE (gpg_err_code_from_errno)
MARK_VISIBLE (gpg_err_code_to_errno)
MARK_VISIBLE (gpg_err_code_from_syserror)
MARK_VISIBLE (gpg_err_set_errno)

MARK_VISIBLE (gpg_err_init)
MARK_VISIBLE (gpg_err_deinit)
MARK_VISIBLE (gpg_error_check_version)
MARK_VISIBLE (gpgrt_check_version)

MARK_VISIBLE (gpgrt_lock_init)
MARK_VISIBLE (gpgrt_lock_lock)
MARK_VISIBLE (gpgrt_lock_unlock)
MARK_VISIBLE (gpgrt_lock_destroy)
MARK_VISIBLE (gpgrt_yield)
MARK_VISIBLE (gpgrt_lock_trylock)

MARK_VISIBLE (gpgrt_fopen)
MARK_VISIBLE (gpgrt_mopen)
MARK_VISIBLE (gpgrt_fopenmem)
MARK_VISIBLE (gpgrt_fopenmem_init)
MARK_VISIBLE (gpgrt_fdopen)
MARK_VISIBLE (gpgrt_fdopen_nc)
MARK_VISIBLE (gpgrt_sysopen)
MARK_VISIBLE (gpgrt_sysopen_nc)
MARK_VISIBLE (gpgrt_fpopen)
MARK_VISIBLE (gpgrt_fpopen_nc)
MARK_VISIBLE (gpgrt_freopen)
MARK_VISIBLE (gpgrt_fopencookie)
MARK_VISIBLE (gpgrt_fclose)
MARK_VISIBLE (gpgrt_fclose_snatch)
MARK_VISIBLE (gpgrt_onclose)
MARK_VISIBLE (gpgrt_fileno)
MARK_VISIBLE (gpgrt_fileno_unlocked)
MARK_VISIBLE (gpgrt_syshd)
MARK_VISIBLE (gpgrt_syshd_unlocked)
MARK_VISIBLE (_gpgrt_set_std_fd)
MARK_VISIBLE (_gpgrt_get_std_stream)
MARK_VISIBLE (gpgrt_flockfile)
MARK_VISIBLE (gpgrt_ftrylockfile)
MARK_VISIBLE (gpgrt_funlockfile)
MARK_VISIBLE (_gpgrt_pending)
MARK_VISIBLE (_gpgrt_pending_unlocked)
MARK_VISIBLE (gpgrt_feof)
MARK_VISIBLE (gpgrt_feof_unlocked)
MARK_VISIBLE (gpgrt_ferror)
MARK_VISIBLE (gpgrt_ferror_unlocked)
MARK_VISIBLE (gpgrt_clearerr)
MARK_VISIBLE (gpgrt_clearerr_unlocked)
MARK_VISIBLE (gpgrt_fflush)
MARK_VISIBLE (gpgrt_fseek)
MARK_VISIBLE (gpgrt_fseeko)
MARK_VISIBLE (gpgrt_ftell)
MARK_VISIBLE (gpgrt_ftello)
MARK_VISIBLE (gpgrt_rewind)
MARK_VISIBLE (gpgrt_fgetc)
MARK_VISIBLE (_gpgrt_getc_underflow)
MARK_VISIBLE (gpgrt_fputc)
MARK_VISIBLE (_gpgrt_putc_overflow)
MARK_VISIBLE (gpgrt_ungetc)
MARK_VISIBLE (gpgrt_read)
MARK_VISIBLE (gpgrt_write)
MARK_VISIBLE (gpgrt_write_sanitized)
MARK_VISIBLE (gpgrt_write_hexstring)
MARK_VISIBLE (gpgrt_fread)
MARK_VISIBLE (gpgrt_fwrite)
MARK_VISIBLE (gpgrt_fgets)
MARK_VISIBLE (gpgrt_fputs)
MARK_VISIBLE (gpgrt_fputs_unlocked)
MARK_VISIBLE (gpgrt_getline)
MARK_VISIBLE (gpgrt_read_line)
MARK_VISIBLE (gpgrt_free)
MARK_VISIBLE (gpgrt_fprintf)
MARK_VISIBLE (gpgrt_fprintf_unlocked)
MARK_VISIBLE (gpgrt_printf)
MARK_VISIBLE (gpgrt_printf_unlocked)
MARK_VISIBLE (gpgrt_vfprintf)
MARK_VISIBLE (gpgrt_vfprintf_unlocked)
MARK_VISIBLE (gpgrt_setvbuf)
MARK_VISIBLE (gpgrt_setbuf)
MARK_VISIBLE (gpgrt_set_binary)
MARK_VISIBLE (gpgrt_tmpfile)
MARK_VISIBLE (gpgrt_opaque_set)
MARK_VISIBLE (gpgrt_opaque_get)
MARK_VISIBLE (gpgrt_fname_set)
MARK_VISIBLE (gpgrt_fname_get)
MARK_VISIBLE (gpgrt_asprintf)
MARK_VISIBLE (gpgrt_vasprintf)
MARK_VISIBLE (gpgrt_bsprintf)
MARK_VISIBLE (gpgrt_vbsprintf)
MARK_VISIBLE (gpgrt_snprintf)
MARK_VISIBLE (gpgrt_vsnprintf)
MARK_VISIBLE (gpgrt_set_syscall_clamp)
MARK_VISIBLE (gpgrt_set_alloc_func)

#undef MARK_VISIBLE

#else /*!_GPGRT_INCL_BY_VISIBILITY_C*/

/* To avoid accidental use of the public functions inside Libgpg-error,
   we redefine them to catch such errors.  */

#define gpg_strerror                _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpg_strerror_r              _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpg_strsource               _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpg_err_code_from_errno     _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpg_err_code_to_errno       _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpg_err_code_from_syserror  _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpg_err_set_errno           _gpgrt_USE_UNDERSCORED_FUNCTION

#define gpg_err_init                _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpg_err_deinit              _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpg_error_check_version     _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_check_version         _gpgrt_USE_OTHER_FUNCTION

#define gpgrt_lock_init             _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_lock_lock             _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_lock_unlock           _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_lock_destroy          _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_yield                 _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_lock_trylock          _gpgrt_USE_UNDERSCORED_FUNCTION

#define gpgrt_fopen                 _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_mopen                 _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_fopenmem              _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_fopenmem_init         _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_fdopen                _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_fdopen_nc             _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_sysopen               _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_sysopen_nc            _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_fpopen                _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_fpopen_nc             _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_freopen               _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_fopencookie           _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_fclose                _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_fclose_snatch         _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_onclose               _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_fileno                _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_fileno_unlocked       _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_syshd                 _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_syshd_unlocked        _gpgrt_USE_UNDERSCORED_FUNCTION
#define _gpgrt_set_std_fd           _gpgrt_USE_UNDERSCORED_FUNCTION
#define _gpgrt_get_std_stream       _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_flockfile             _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_ftrylockfile          _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_funlockfile           _gpgrt_USE_UNDERSCORED_FUNCTION
#define _gpgrt_pending              _gpgrt_USE_UNDERSCORED_FUNCTION
#define _gpgrt_pending_unlocked     _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_feof                  _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_feof_unlocked         _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_ferror                _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_ferror_unlocked       _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_clearerr              _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_clearerr_unlocked     _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_fflush                _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_fseek                 _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_fseeko                _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_ftell                 _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_ftello                _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_rewind                _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_fgetc                 _gpgrt_USE_UNDERSCORED_FUNCTION
#define _gpgrt_getc_underflow       _gpgrt_USE_DBLUNDERSCO_FUNCTION
#define gpgrt_fputc                 _gpgrt_USE_UNDERSCORED_FUNCTION
#define _gpgrt_putc_overflow        _gpgrt_USE_DBLUNDERSCO_FUNCTION
#define gpgrt_ungetc                _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_read                  _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_write                 _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_write_sanitized       _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_write_hexstring       _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_fread                 _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_fwrite                _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_fgets                 _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_fputs                 _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_fputs_unlocked        _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_getline               _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_read_line             _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_free                  _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_fprintf               _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_fprintf_unlocked      _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_printf                _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_printf_unlocked       _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_vfprintf              _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_vfprintf_unlocked     _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_setvbuf               _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_setbuf                _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_set_binary            _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_tmpfile               _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_opaque_set            _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_opaque_get            _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_fname_set             _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_fname_get             _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_asprintf              _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_vasprintf             _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_bsprintf              _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_vbsprintf             _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_snprintf              _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_vsnprintf             _gpgrt_USE_UNDERSCORED_FUNCTION

#define gpgrt_set_syscall_clamp     _gpgrt_USE_UNDERSCORED_FUNCTION
#define gpgrt_set_alloc_func        _gpgrt_USE_UNDERSCORED_FUNCTION


#endif /*!_GPGRT_INCL_BY_VISIBILITY_C*/

#endif /*_GPGRT_VISIBILITY_H*/
