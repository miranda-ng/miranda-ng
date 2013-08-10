/*
	Pcre.cpp
	Copyright (c) 2007-2008 Chervov Dmitry

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "../Common.h"
#include <windows.h>
#include <stdio.h>
#include <crtdbg.h>
#include "newpluginapi.h"
#include "m_utils.h"
#include "TMyArray.h"
#include "CString.h"
#include "pcre.h"


HMODULE hPcreDLL = NULL;

static pcre* (*pcre_compile)(const char*, int, const char**, int*, const unsigned char*);
static int (*pcre_config)(int, void*);
static int (*pcre_exec)(const pcre*, const pcre_extra*, const char*, int, int, int, int*, int);
static void (*pcre_free)(void*);
static pcre_extra* (*pcre_study)(const pcre*, int, const char **);


typedef struct
{
	pcre *pPcre;
	pcre_extra *pExtra;
	TCString Pattern; // used when it's not a valid regexp
	int ID; // user-defined ID of the pattern; returned by PcreCheck on a match
} sPcreCompileData;

TMyArray<sPcreCompileData> PcreCompileData;


void FreePcreCompileData()
{
	int I;
	for (I = 0; I < PcreCompileData.GetSize(); I++)
	{
		if (PcreCompileData[I].pPcre)
		{
			pcre_free(PcreCompileData[I].pPcre);
			if (PcreCompileData[I].pExtra)
			{
				pcre_free(PcreCompileData[I].pExtra);
			}
		}
	}
	PcreCompileData.RemoveAll();
}


TCString CompileRegexp(TCString Regexp, int bAddAsUsualSubstring, int ID)
{
	TCString Result(_T(""));
	sPcreCompileData s = {0};
	int NewID = PcreCompileData.AddElem(s);
	PcreCompileData[NewID].ID = ID;
	if (hPcreDLL && !bAddAsUsualSubstring)
	{
		const char *Err;
		int ErrOffs;
		int Flags = PCRE_CASELESS;
		if (Regexp[0] == '/')
		{
			TCString OrigRegexp = Regexp;
			Regexp = Regexp.Right(Regexp.GetLen() - 1);
			TCHAR *pRegexpEnd = (TCHAR*)Regexp + Regexp.GetLen();
			TCHAR *p = _tcsrchr(Regexp.GetBuffer(), '/');
			if (!p)
			{
				Regexp = OrigRegexp;
			} else
			{
				*p = 0;
				Flags = 0;
				while (++p < pRegexpEnd)
				{
					switch (*p) {
					case 'i':
						Flags |= PCRE_CASELESS;
						break;
					case 'm':
						Flags |= PCRE_MULTILINE;
						break;
					case 's':
						Flags |= PCRE_DOTALL;
						break;
					case 'x':
						Flags |= PCRE_EXTENDED;
						break;
					case 'A':
						Flags |= PCRE_ANCHORED;
						break;
					case 'f':
						Flags |= PCRE_FIRSTLINE;
						break;
					case 'D':
						Flags |= PCRE_DOLLAR_ENDONLY;
						break;
					case 'U':
						Flags |= PCRE_UNGREEDY;
						break;
					case 'X':
						Flags |= PCRE_EXTRA;
						break;
					default:
						// Result += LogMessage(Translate("Warning, unknown pattern modifier '%c':\n"), *p );
						break;
					}
				}
			}
			Regexp.ReleaseBuffer();
		}

		PcreCompileData[NewID].pPcre = pcre_compile(WCHAR2UTF8(Regexp).GetData(), PCRE_UTF8 | PCRE_NO_UTF8_CHECK | Flags, &Err, &ErrOffs, NULL);

		if (PcreCompileData[NewID].pPcre) {
			PcreCompileData[NewID].pExtra = NULL;
			if (pcre_study)
				PcreCompileData[NewID].pExtra = pcre_study(PcreCompileData[NewID].pPcre, 0, &Err);
		} 
		else {
			// Result += LogMessage(TranslateT("Syntax error in regexp\n%s\nat offset %d: %s."), (TCHAR*)Regexp, ErrOffs, (TCHAR*)ANSI2TCHAR(Err)) + _T("\n\n");
			PcreCompileData[NewID].Pattern = Regexp;
	 	}
	} 
	else PcreCompileData[NewID].Pattern = Regexp;

	return Result;
}


HMODULE LoadPcreLibrary(const char *szPath)
{
	_ASSERT(szPath);
	HMODULE hModule = LoadLibraryA(szPath);
	if (!hModule)
	{
		return NULL;
	}
	*(FARPROC*)&pcre_config = GetProcAddress(hModule, "pcre_config");
	*(FARPROC*)&pcre_compile = GetProcAddress(hModule, "pcre_compile");
	*(FARPROC*)&pcre_exec = GetProcAddress(hModule, "pcre_exec");
	*(FARPROC*)&pcre_study = GetProcAddress(hModule, "pcre_study");
	*(FARPROC*)&pcre_free = *(FARPROC*)GetProcAddress(hModule, "pcre_free"); // pcre_free is a pointer to a variable containing pointer to the function %)
	if (pcre_compile && pcre_exec && pcre_free)
	{

		int Utf8Supported = 0;
		if (pcre_config)
		{
			pcre_config(PCRE_CONFIG_UTF8, &Utf8Supported);
		}
		if (Utf8Supported)
		{
			return hModule;
		}

	}
	FreeLibrary(hModule);
	return NULL;
}


void InitPcre()
{
	_ASSERT(!hPcreDLL);
	hPcreDLL = LoadPcreLibrary("pcre.dll");
	if (!hPcreDLL)
	{
		hPcreDLL = LoadPcreLibrary("pcre3.dll");
	}
	if (!hPcreDLL)
	{
		char path[MAX_PATH];
		GetModuleFileNameA(NULL, path, sizeof(path));
		char *p = strrchr(path, '\\');
		if (p)
		{
			strcpy(p + 1, "pcre.dll");
		} else
		{
			strcpy(path, "pcre.dll");
		}
		hPcreDLL = LoadPcreLibrary(path);
		if (!hPcreDLL)
		{
			if (p)
			{
				strcpy(p + 1, "pcre3.dll");
			} else
			{
				strcpy(path, "pcre3.dll");
			}
			hPcreDLL = LoadPcreLibrary(path);
		}
	}
}


void UninitPcre()
{
	if (hPcreDLL)
	{
		FreePcreCompileData();
		FreeLibrary(hPcreDLL);
	}
}


int PcreEnabled()
{
	return (int)hPcreDLL;
}


int PcreCheck(TCString Str, int StartingID)
{ // StartingID specifies the pattern from which to start checking, i.e. the check starts from the next pattern after the one that has ID == StartingID
	int I;
	if (StartingID == -1)
	{
		I = 0;
	} else
	{
		for (I = 0; I < PcreCompileData.GetSize(); I++)
		{
			if (PcreCompileData[I].ID == StartingID)
			{
				I++;
				break;
			}
		}
	}
	for (; I < PcreCompileData.GetSize(); I++)
	{
		if (hPcreDLL && PcreCompileData[I].pPcre)
		{

			CHARARRAY Utf8Str = WCHAR2UTF8(Str);
			int Res = pcre_exec(PcreCompileData[I].pPcre, PcreCompileData[I].pExtra, Utf8Str.GetData(), Utf8Str.GetSize() - 1, 0, PCRE_NOTEMPTY | PCRE_NO_UTF8_CHECK, NULL, 0);
			
			if (Res >= 0)
			{
				return PcreCompileData[I].ID;
			}
		} else
		{
			if (_tcsstr(Str.ToLower(), PcreCompileData[I].Pattern.ToLower()))
			{
				return PcreCompileData[I].ID;
			}
		}
	}
	return -1;
}
