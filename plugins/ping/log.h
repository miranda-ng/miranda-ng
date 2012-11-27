#ifndef _PING_LOG
#define _PING_LOG

#pragma warning( disable : 4786 )
#include "options.h"

INT_PTR Log(WPARAM wParam, LPARAM lParam);
INT_PTR GetLogFilename(WPARAM wParam, LPARAM lParam);
INT_PTR SetLogFilename(WPARAM wParam, LPARAM lParam);
INT_PTR ViewLogData(WPARAM wParam, LPARAM lParam);

#endif
