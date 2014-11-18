// System includes
#include <stdio.h>
#include <windows.h>
#include <commctrl.h>
#include <process.h>
#include <time.h>
#include "resource.h"
#include "skype.h"

// Miranda database access
#include <newpluginapi.h>
#include <m_database.h>

typedef struct
{
	TCHAR FullName[256];
	char HomePhone[256];
	char OfficePhone[256];
	char HomePage[256];
	TCHAR City[256];
	TCHAR Province[256];
	BYTE Sex;
	SYSTEMTIME Birthday;
} SkypeProfile;

void SkypeProfile_Load(SkypeProfile *pstProf);
void SkypeProfile_Save(SkypeProfile *pstProf);
void SkypeProfile_Free(SkypeProfile *pstProf);
void SkypeProfile_LoadFromSkype(SkypeProfile *pstProf);
void SkypeProfile_SaveToSkype(SkypeProfile *pstProf);
