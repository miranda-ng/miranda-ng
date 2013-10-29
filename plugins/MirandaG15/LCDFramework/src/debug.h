#ifndef _DEBUG_H_
#define _DEBUG_H_

#define USECONSTREAM 1

extern void InitDebug();
extern void UnInitDebug();


#ifdef _DEBUG
#ifdef USECONSTREAM
	#include "ConStream.h"
#endif
#define TRACE _trace
extern void _trace(TCHAR *fmt, ...);
#else
inline void _trace(LPCTSTR fmt, ...) { }
#define TRACE  1 ? (void)0 : _trace
#endif

#endif