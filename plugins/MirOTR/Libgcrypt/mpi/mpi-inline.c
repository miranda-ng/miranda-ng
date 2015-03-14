/* mpi-inline.c
 *	Copyright (C) 1999, 2002 Free Software Foundation, Inc.
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>

/* put the inline functions as real functions into the lib */
#define G10_MPI_INLINE_DECL

#include "mpi-internal.h"

/* always include the header becuase it is only
 * included by mpi-internal if __GCC__ is defined but we
 * need it here in all cases and the above definition of
 * of the macro allows us to do so
 */
#include "mpi-inline.h"

