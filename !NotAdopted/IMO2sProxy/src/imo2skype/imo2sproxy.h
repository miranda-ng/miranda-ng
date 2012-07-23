#include <stdio.h>
#include "imo2skypeapi.h"
#ifndef WIN32
#define __cdecl
#endif

typedef struct
{
	int bVerbose;
	FILE *fpLog;
	int iFlags;
	int (__cdecl *logerror)( FILE *stream, const char *format, ...);
// FIXME: user+pass from cmdline, until there is a possibility for 
// a client to authenticate
	char *pszUser;
	char *pszPass;
} IMO2SPROXY_CFG;

typedef struct tag_imo2sproxy IMO2SPROXY;

struct tag_imo2sproxy
{
	int  (*Open)(IMO2SPROXY*);
	void (*Loop)(IMO2SPROXY*);
	void (*Exit)(IMO2SPROXY*);
};

void Imo2sproxy_Defaults (IMO2SPROXY_CFG *pCfg);
