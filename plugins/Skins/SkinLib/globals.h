#ifndef __GLOBALS_H__
# define __GLOBALS_H__

#ifdef DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include <windows.h>
#include <newpluginapi.h>
#include <m_system.h>
#include <m_utils.h>
#include <utf8_helpers.h>


#define MAX_REGS(_X_) ( sizeof(_X_) / sizeof(_X_[0]) )


#endif // __GLOBALS_H__