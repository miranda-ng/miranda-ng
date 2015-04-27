#pragma once
#ifndef _INC_DEBUG
#define _INC_DEBUG

#include <windows.h>
#include <stdio.h>
#include <tchar.h>

void _OutputDebugString(TCHAR* lpOutputString, ...);
#endif