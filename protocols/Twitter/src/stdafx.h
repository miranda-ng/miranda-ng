// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef _WIN32_WINNT            // Specifies that the minimum required platform is Windows Vista.
#define _WIN32_WINNT 0x0600     // Change this to the appropriate value to target other versions of Windows.
#endif

#include <Windows.h>
#include <Shlwapi.h>
#include <Wincrypt.h>
#include <stdio.h>
#include <tchar.h>
#include <time.h>

#include <string>
#include <list>
#include <vector>
#include <map>
#include <fstream>

typedef std::basic_string<TCHAR> tstring;

#include "StringConv.h"
#include "StringUtil.h"

#include "Debug.h"

#include "win2k.h"
