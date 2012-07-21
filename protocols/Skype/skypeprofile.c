#pragma warning (disable: 4706) // assignment within conditional expression

#include "skypeprofile.h"
#include "skypeapi.h"
#include "utf8.h"

extern char g_szProtoName[];

void SkypeProfile_Save(SkypeProfile *pstProf)
{
	DBWriteContactSettingByte(NULL, SKYPE_PROTONAME, "Gender", pstProf->Sex);
	DBWriteContactSettingString(NULL, SKYPE_PROTONAME, "HomePhone", pstProf->HomePhone);
	DBWriteContactSettingString(NULL, SKYPE_PROTONAME, "OfficePhone", pstProf->OfficePhone);
	DBWriteContactSettingString(NULL, SKYPE_PROTONAME, "HomePage", pstProf->HomePage);
	DBWriteContactSettingTString(NULL, SKYPE_PROTONAME, "Nick", pstProf->FullName);
	DBWriteContactSettingTString(NULL, SKYPE_PROTONAME, "City", pstProf->City);
	DBWriteContactSettingTString(NULL, SKYPE_PROTONAME, "Province", pstProf->Province);
	DBWriteContactSettingWord(NULL, SKYPE_PROTONAME, "BirthYear", (WORD)pstProf->Birthday.wYear);
	DBWriteContactSettingByte(NULL, SKYPE_PROTONAME, "BirthMonth", (BYTE)pstProf->Birthday.wMonth);
	DBWriteContactSettingByte(NULL, SKYPE_PROTONAME, "BirthDay", (BYTE)pstProf->Birthday.wDay);
}

void SkypeProfile_Load(SkypeProfile *pstProf)
{
	DBVARIANT dbv;

	pstProf->Sex = (BYTE)DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "Gender", 0);
	pstProf->Birthday.wYear = (WORD)DBGetContactSettingWord(NULL, SKYPE_PROTONAME, "BirthYear", 1900);
	pstProf->Birthday.wMonth = (WORD)DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "BirthMonth", 01);
	pstProf->Birthday.wDay = (WORD)DBGetContactSettingByte(NULL, SKYPE_PROTONAME, "BirthDay", 01);
	if(!DBGetContactSettingTString(NULL,SKYPE_PROTONAME,"Nick",&dbv)) 
	{	
		_tcsncpy (pstProf->FullName, dbv.ptszVal, sizeof(pstProf->FullName)/sizeof(TCHAR));
		DBFreeVariant(&dbv);
	}
	if(!DBGetContactSettingString(NULL,SKYPE_PROTONAME,"HomePage",&dbv)) 
	{	
		strncpy (pstProf->HomePage, dbv.pszVal, sizeof(pstProf->HomePage));
		DBFreeVariant(&dbv);
	}
	if(!DBGetContactSettingTString(NULL,SKYPE_PROTONAME,"Province",&dbv)) 
	{	
		_tcsncpy (pstProf->Province, dbv.ptszVal, sizeof(pstProf->Province)/sizeof(TCHAR));
		DBFreeVariant(&dbv);
	}
	if(!DBGetContactSettingTString(NULL,SKYPE_PROTONAME,"City",&dbv)) 
	{	
		_tcsncpy (pstProf->City, dbv.ptszVal, sizeof(pstProf->City)/sizeof(TCHAR));
		DBFreeVariant(&dbv);
	}
	if(!DBGetContactSettingString(NULL,SKYPE_PROTONAME,"OfficePhone",&dbv)) 
	{	
		strncpy (pstProf->OfficePhone, dbv.pszVal, sizeof(pstProf->OfficePhone));
		DBFreeVariant(&dbv);
	}
	if(!DBGetContactSettingString(NULL,SKYPE_PROTONAME,"HomePhone",&dbv)) 
	{	
		strncpy (pstProf->HomePhone, dbv.pszVal, sizeof(pstProf->HomePhone));
		DBFreeVariant(&dbv);
	}
}

static void LoadSaveSkype(SkypeProfile *pstProf, BOOL bSet)
{
#pragma warning (push)
#pragma warning (disable: 4204) // nonstandard extension used : non-constant aggregate initializer
#define ENTRY(x,y) {x, pstProf->y, sizeof(pstProf->y)/sizeof(pstProf->y[0]), sizeof(pstProf->y[0])}
	const struct {
		char *pszSetting;
		LPVOID lpDest;
		int iSize;
		char cType;
	} astSettings[] = {
		ENTRY("FULLNAME", FullName),
		ENTRY("PHONE_HOME", HomePhone),
		ENTRY("PHONE_OFFICE", OfficePhone),
		ENTRY("HOMEPAGE", HomePage),
		ENTRY("CITY", City),
		ENTRY("PROVINCE", Province)
	};
#pragma warning (pop)
#undef ENTRY
	char *ptr;
	int i;

	if (bSet) {
		char *pBuf, szBirthday[16];
		for (i=0; i<sizeof(astSettings)/sizeof(astSettings[0]); i++) {
			if ((astSettings[i].cType == sizeof(char)  && utf8_encode((const char*)astSettings[i].lpDest, &pBuf) != -1) ||
				(astSettings[i].cType == sizeof(WCHAR) && (pBuf = (char*)make_utf8_string((const WCHAR*)astSettings[i].lpDest)))) {
					SkypeSetProfile (astSettings[i].pszSetting, pBuf);
					free (pBuf);
			}
		}
		switch (pstProf->Sex)
		{
		case 0x4D: SkypeSetProfile ("SEX", "MALE"); break;
		case 0x46: SkypeSetProfile ("SEX", "FEMALE"); break;
		}
		sprintf (szBirthday, "%04d%02d%02d", pstProf->Birthday.wYear, pstProf->Birthday.wMonth, pstProf->Birthday.wDay);
		SkypeSetProfile ("BIRTHDAY", szBirthday);
	} else {
		for (i=0; i<sizeof(astSettings)/sizeof(astSettings[0]); i++) {
			if (ptr=SkypeGetProfile(astSettings[i].pszSetting)) {
				if (astSettings[i].cType == sizeof(char)) {
					char *pBuf;
					if (utf8_decode (ptr, &pBuf) != -1) {
						strncpy ((char*)astSettings[i].lpDest, pBuf, astSettings[i].iSize);
						free (pBuf);
					}
				} else {
					WCHAR *pBuf;
					if (pBuf = make_unicode_string((const unsigned char*)ptr)) {
						wcsncpy ((WCHAR*)astSettings[i].lpDest, pBuf, astSettings[i].iSize);
						free (pBuf);
					}
				}
				free (ptr);
			}
		}
		if (ptr=SkypeGetProfile("SEX"))
		{
			if (!_stricmp(ptr, "MALE")) pstProf->Sex=0x4D; else
			if (!_stricmp(ptr, "FEMALE")) pstProf->Sex=0x46;
			free (ptr);
		}
		if (ptr=SkypeGetProfile("BIRTHDAY"))
		{
			if (*ptr != '0')
				sscanf(ptr, "%04hd%02hd%02hd", &pstProf->Birthday.wYear, &pstProf->Birthday.wMonth, 
					&pstProf->Birthday.wDay);
			free(ptr);
		}
	}
}

void SkypeProfile_LoadFromSkype(SkypeProfile *pstProf)
{
	LoadSaveSkype (pstProf, FALSE);
}

void SkypeProfile_SaveToSkype(SkypeProfile *pstProf)
{
	LoadSaveSkype (pstProf, TRUE);
}