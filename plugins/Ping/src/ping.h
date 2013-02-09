/*
Based on the
Miranda plugin template, originally by Richard Hughes
http://miranda-icq.sourceforge.net/

© 2004 Scott Ellis

*/

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the PINGPROTO_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// PINGPROTO_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.

#include "utils.h"
#include "options.h"
#include "pinglist.h"
#include "log.h"
#include "pingthread.h"
#include "menu.h"
#include "rawping.h"
