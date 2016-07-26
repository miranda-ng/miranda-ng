/****h* Win32/luaw32.c [$Revision: 13 $]
* NAME
*  luaw32
* COPYRIGHT
*  (C) 2004-2007 Daniel Quintela.  All rights reserved.
*  http://www.soongsoft.com mailto:dq@soongsoft.com
*  (C) 2013 http://quik2dde.ru
* LICENSE
*  Permission is hereby granted, free of charge, to any person obtaining
*  a copy of this software and associated documentation files (the
*  "Software"), to deal in the Software without restriction, including
*  without limitation the rights to use, copy, modify, merge, publish,
*  distribute, sublicense, and/or sell copies of the Software, and to
*  permit persons to whom the Software is furnished to do so, subject to
*  the following conditions:
*
*  The above copyright notice and this permission notice shall be
*  included in all copies or substantial portions of the Software.
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
*  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
*  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
* FUNCTION
*  Win32 functions & constants binding..
* AUTHOR
*  Daniel Quintela
* CREATION DATE
*  2004/08/16
* MODIFICATION HISTORY
*  $Id: luaw32.c 13 2007-04-15 13:39:04Z  $
*
*        When                Who                      What
* -------------------  ----------------  ----------------------------------------
* 2004-09-01 12:20:00  Danilo Tuler      GetTempPath added.
* 2006-08-25 08:20:00  Denis Povshedny   QueryServiceConfig & StartService added.
* 2007-04-15 10:31:00  Daniel Quintela   CreateMutex parameter list fixed.
* 2013-10-27 00:00:15  www.quik2dde.ru   Linking with qlia.dll
*
* NOTES
*
***/

#define LUA_WINAPI_LIB extern "C" int __declspec(dllexport)

#include <lua.hpp>

#include <windows.h>
#include <stddef.h>
#include <process.h>
#include <direct.h>
#include <shlwapi.h>
#include <shlobj.h>

#include <time.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utime.h>

#include <m_core.h>
#include <m_utils.h>

struct luaM_consts
{
	char    *name;
	intptr_t   value;
};


#define luaM_PushNumberIf(L, a, b) if (b) lua_pushnumber(L, (intptr_t)a); else lua_pushnil(L);
#define luaM_PushStringIf(L, a, b) if (b) lua_pushstring(L, a); else lua_pushnil(L);
#define luaM_CheckPushNumber(L, a) luaM_PushNumberIf(L, a, a)