// stdafx.h : Includedatei für Standardsystem-Includedateien
// oder häufig verwendete projektspezifische Includedateien,
// die nur in unregelmäßigen Abständen geändert werden.
//

#pragma once

#ifndef _WIN32_WINNT		// Lassen Sie die Verwendung spezifischer Features von Windows XP oder später zu.                   
#define _WIN32_WINNT 0x0501	// Ändern Sie dies in den geeigneten Wert für andere Versionen von Windows.
#endif						

#include <stdio.h>
#include <cstdlib>
#include <tchar.h>
#include <iostream>

#define uint unsigned int
#define socklen_t int

#define NO_PTHREAD

#pragma warning(disable:4267)
#pragma warning(disable:4244)
#pragma warning(disable:4312)
#pragma warning(disable:4099)
#pragma warning(disable:4311)
#pragma comment(lib,"ws2_32.lib")

#ifndef NO_PTHREAD
#pragma comment(lib,"pthreads.lib")
#endif

#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")

// TODO: Hier auf zusätzliche Header, die das Programm erfordert, verweisen.
