/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef M_STDHDR_H__
#define M_STDHDR_H__ 1

#if defined( UNICODE ) && !defined( _UNICODE )
#  define _UNICODE
#endif

#ifndef _WIN64
#define _USE_32BIT_TIME_T
#endif

#include <tchar.h>

#if _MSC_VER < 1400
#	include <malloc.h>   // to avoid a crazy bug in VS2003 header files
#endif

#ifndef __GNUC__
#	ifdef _DEBUG
#		define _CRTDBG_MAP_ALLOC
#		include <stdlib.h>
#		include <crtdbg.h>
#       define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#  else
#		include <stdlib.h>
#	endif
#else
#	include <stdlib.h>
/* GCC has not __try/except */
#	define __try
#	define __except(x) if (0) /* don't execute handler */
#	define __finally
/* neither _ASSERT */
#	ifdef _DEBUG
#		include <assert.h>
#		define _ASSERT assert
#	else
#		define _ASSERT(x)
#	endif
/* neither some CRT debugging things */
#	define _CRTDBG_REPORT_FLAG -1
#	define _CRTDBG_LEAK_CHECK_DF 0x20
#	define _CrtSetDbgFlag(x) 0
#endif

#if _MSC_VER >= 1400
#	include <malloc.h>   // to avoid a warning in VS2005 & 2008
#endif


#endif // M_STDHDR_H__
