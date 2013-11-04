#ifndef _LCDFRAMEWORK_H
#define _LCDFRAMEWORK_H

// HID Includes
#include <setupapi.h>
extern "C"
{
#include "hid/hidsdi.h"
}

#ifdef _UNICODE
#define tstring wstring
#else
#define tstring string
#endif

#include "debug.h"
#include "misc.h"

#endif