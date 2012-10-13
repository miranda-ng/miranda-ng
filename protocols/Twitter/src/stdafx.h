// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <Windows.h>
//#include <WinInet.h>
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
//#define SIZEOF(x) (sizeof(x)/sizeof(*x))

#include "StringConv.h"
#include "StringUtil.h"
#include "Base64Coder.h"

#include "Debug.h"

#include "win2k.h"
