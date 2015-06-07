#ifndef _COMMON_H_
#define _COMMON_H_

#include <windows.h>

#include <newpluginapi.h>
#include <m_langpack.h>

extern "C"
{
	#include "lua\lua.h"
	#include "lua\lualib.h"
	#include "lua\lauxlib.h"
}

#include "version.h"
#include "resource.h"

class CMLua;

#include "mlua.h"

#define MODULE "MirLua"

extern HINSTANCE g_hInstance;

#endif //_COMMON_H_
