#ifndef _STDAFX_H_
#define _STDAFX_H_

#include <assert.h>

#define ASSERT assert

#include <Windows.h>
#include <tchar.h>
#include <string>
#include <vector>
#include <queue>
#include <list>
using namespace std;

#ifdef _UNICODE
	#define tstring wstring
#else
	#define tstring string
#endif

#include "debug.h"
#include "misc.h"

// HID Includes
#include <setupapi.h>
extern "C"
{
#include "hidsdi.h"
}

#endif