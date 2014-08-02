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

typedef struct
{
	pcre16 *pPcre;
	pcre16_extra *pExtra;
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
			pcre16_free(PcreCompileData[I].pPcre);
			if (PcreCompileData[I].pExtra)
			{
				pcre16_free(PcreCompileData[I].pExtra);
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
	if (!bAddAsUsualSubstring)
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

		PcreCompileData[NewID].pPcre = pcre16_compile(Regexp, PCRE_UTF8 | PCRE_NO_UTF8_CHECK | Flags, &Err, &ErrOffs, NULL);

		if (PcreCompileData[NewID].pPcre) {
			PcreCompileData[NewID].pExtra = NULL;
			PcreCompileData[NewID].pExtra = pcre16_study(PcreCompileData[NewID].pPcre, 0, &Err);
		} 
		else {
			// Result += LogMessage(TranslateT("Syntax error in regexp\n%s\nat offset %d: %s."), (TCHAR*)Regexp, ErrOffs, (TCHAR*)ANSI2TCHAR(Err)) + _T("\n\n");
			PcreCompileData[NewID].Pattern = Regexp;
	 	}
	} 
	else PcreCompileData[NewID].Pattern = Regexp;

	return Result;
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
		if (PcreCompileData[I].pPcre)
		{

			int Res = pcre16_exec(PcreCompileData[I].pPcre, PcreCompileData[I].pExtra, Str, Str.GetLen() - 1, 0, PCRE_NOTEMPTY | PCRE_NO_UTF8_CHECK, NULL, 0);
			
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
