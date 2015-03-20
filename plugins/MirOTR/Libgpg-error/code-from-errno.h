/* Output of mkerrcodes2.awk.  DO NOT EDIT.  */

/* errnos.h - List of system error values.
   Copyright (C) 2004 g10 Code GmbH
   This file is part of libgpg-error.

   libgpg-error is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.

   libgpg-error is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with libgpg-error; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

static const int err_code_from_index[] = {
  GPG_ERR_EPERM,
  GPG_ERR_ENOENT,
  GPG_ERR_ESRCH,
  GPG_ERR_EINTR,
  GPG_ERR_EIO,
  GPG_ERR_ENXIO,
  GPG_ERR_E2BIG,
  GPG_ERR_ENOEXEC,
  GPG_ERR_EBADF,
  GPG_ERR_ECHILD,
  GPG_ERR_EAGAIN,
  GPG_ERR_ENOMEM,
  GPG_ERR_EACCES,
  GPG_ERR_EFAULT,
  GPG_ERR_EBUSY,
  GPG_ERR_EEXIST,
  GPG_ERR_EXDEV,
  GPG_ERR_ENODEV,
  GPG_ERR_ENOTDIR,
  GPG_ERR_EISDIR,
  GPG_ERR_EINVAL,
  GPG_ERR_ENFILE,
  GPG_ERR_EMFILE,
  GPG_ERR_ENOTTY,
  GPG_ERR_EFBIG,
  GPG_ERR_ENOSPC,
  GPG_ERR_ESPIPE,
  GPG_ERR_EROFS,
  GPG_ERR_EMLINK,
  GPG_ERR_EPIPE,
  GPG_ERR_EDOM,
  GPG_ERR_ERANGE,
  GPG_ERR_EDEADLK,
  GPG_ERR_EDEADLOCK,
  GPG_ERR_ENAMETOOLONG,
  GPG_ERR_ENOLCK,
  GPG_ERR_ENOSYS,
  GPG_ERR_ENOTEMPTY,
  GPG_ERR_EILSEQ,
  GPG_ERR_EADDRINUSE,
  GPG_ERR_EADDRNOTAVAIL,
  GPG_ERR_EAFNOSUPPORT,
  GPG_ERR_EALREADY,
  GPG_ERR_ECANCELED,
  GPG_ERR_ECONNABORTED,
  GPG_ERR_ECONNREFUSED,
  GPG_ERR_ECONNRESET,
  GPG_ERR_EDESTADDRREQ,
  GPG_ERR_EHOSTUNREACH,
  GPG_ERR_EINPROGRESS,
  GPG_ERR_EISCONN,
  GPG_ERR_ELOOP,
  GPG_ERR_EMSGSIZE,
  GPG_ERR_ENETDOWN,
  GPG_ERR_ENETRESET,
  GPG_ERR_ENETUNREACH,
  GPG_ERR_ENOBUFS,
  GPG_ERR_ENOPROTOOPT,
  GPG_ERR_ENOTCONN,
  GPG_ERR_ENOTSOCK,
  GPG_ERR_ENOTSUP,
  GPG_ERR_EOPNOTSUPP,
  GPG_ERR_EOVERFLOW,
  GPG_ERR_EPROTO,
  GPG_ERR_EPROTONOSUPPORT,
  GPG_ERR_EPROTOTYPE,
  GPG_ERR_ETIMEDOUT,
  GPG_ERR_EWOULDBLOCK,
  GPG_ERR_EINTR,
  GPG_ERR_EBADF,
  GPG_ERR_EACCES,
  GPG_ERR_EFAULT,
  GPG_ERR_EINVAL,
  GPG_ERR_EMFILE,
  GPG_ERR_EWOULDBLOCK,
  GPG_ERR_EINPROGRESS,
  GPG_ERR_EALREADY,
  GPG_ERR_ENOTSOCK,
  GPG_ERR_EDESTADDRREQ,
  GPG_ERR_EMSGSIZE,
  GPG_ERR_EPROTOTYPE,
  GPG_ERR_ENOPROTOOPT,
  GPG_ERR_EPROTONOSUPPORT,
  GPG_ERR_ESOCKTNOSUPPORT,
  GPG_ERR_EOPNOTSUPP,
  GPG_ERR_EPFNOSUPPORT,
  GPG_ERR_EAFNOSUPPORT,
  GPG_ERR_EADDRINUSE,
  GPG_ERR_EADDRNOTAVAIL,
  GPG_ERR_ENETDOWN,
  GPG_ERR_ENETUNREACH,
  GPG_ERR_ENETRESET,
  GPG_ERR_ECONNABORTED,
  GPG_ERR_ECONNRESET,
  GPG_ERR_ENOBUFS,
  GPG_ERR_EISCONN,
  GPG_ERR_ENOTCONN,
  GPG_ERR_ESHUTDOWN,
  GPG_ERR_ETOOMANYREFS,
  GPG_ERR_ETIMEDOUT,
  GPG_ERR_ECONNREFUSED,
  GPG_ERR_ELOOP,
  GPG_ERR_ENAMETOOLONG,
  GPG_ERR_EHOSTDOWN,
  GPG_ERR_EHOSTUNREACH,
  GPG_ERR_ENOTEMPTY,
  GPG_ERR_EPROCLIM,
  GPG_ERR_EUSERS,
  GPG_ERR_EDQUOT,
  GPG_ERR_ESTALE,
  GPG_ERR_EREMOTE,
};

#define errno_to_idx(code) (0 ? -1 \
  : ((code >= 1) && (code <= 14)) ? (code - 1) \
  : ((code >= 16) && (code <= 25)) ? (code - 2) \
  : ((code >= 27) && (code <= 34)) ? (code - 3) \
  : ((code >= 36) && (code <= 36)) ? (code - 4) \
  : ((code >= 36) && (code <= 36)) ? (code - 3) \
  : ((code >= 38) && (code <= 42)) ? (code - 4) \
  : ((code >= 100) && (code <= 103)) ? (code - 61) \
  : ((code >= 105) && (code <= 110)) ? (code - 62) \
  : ((code >= 112) && (code <= 119)) ? (code - 63) \
  : ((code >= 123) && (code <= 123)) ? (code - 66) \
  : ((code >= 126) && (code <= 126)) ? (code - 68) \
  : ((code >= 128) && (code <= 130)) ? (code - 69) \
  : ((code >= 132) && (code <= 132)) ? (code - 70) \
  : ((code >= 134) && (code <= 136)) ? (code - 71) \
  : ((code >= 138) && (code <= 138)) ? (code - 72) \
  : ((code >= 140) && (code <= 140)) ? (code - 73) \
  : ((code >= 10004) && (code <= 10004)) ? (code - 9936) \
  : ((code >= 10009) && (code <= 10009)) ? (code - 9940) \
  : ((code >= 10013) && (code <= 10014)) ? (code - 9943) \
  : ((code >= 10022) && (code <= 10022)) ? (code - 9950) \
  : ((code >= 10024) && (code <= 10024)) ? (code - 9951) \
  : ((code >= 10035) && (code <= 10071)) ? (code - 9961) \
  : -1)
