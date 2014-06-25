
//This file is part of Msg_Export a Miranda IM plugin
//Copyright (C)2002 Kennet Nielsen ( http://sourceforge.net/projects/msg-export/ )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "Glob.h"

static UINT UM_FIND_CMD = RegisterWindowMessage( FINDMSGSTRING);

#define ID_FV_FONT			0x0010
#define ID_FV_COLOR			0x0020
#define ID_FV_SYNTAX_HL    0x0030
#define ID_FV_SAVE_AS_RTF  0x0040
//	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
//	ASSERT(IDM_ABOUTBOX < 0xF000);


// Specifies if history is opened internaly or externaly
bool bUseIntViewer = true;

// External program used to view files
tstring sFileViewerPrg;

// handle to the RichEditDll. We need to load this dll to use a RichEdit.
HMODULE hRichEditDll = NULL;


#define CONT(i) ((in[i]&0xc0) == 0x80)
#define VAL(i, s) ((in[i]&0x3f) << s)

void swap(char &c1, char &c2) {
    char ch;
    ch=c1;
    c1=c2;
    c2=ch;
}

int DecodeUTF8(const char *pcBuff,int /*iBufSize*/,char *pcOutBuf) {
	int iBytesInOut=0;
	int /*cp,*/i;
	char ch,*p;

//Parse UTF-8 sequence
//Support only chars up to three bytes (UCS-4 - go away!)
//Warning: Partial decoding is possible!
    i=0;
    ch=pcBuff[i];
    if(!(ch&0x80)) {
        pcOutBuf[iBytesInOut++]=ch;
        pcOutBuf[iBytesInOut++]='\0';
    }
    else if(!(ch&0x20)) {
        pcOutBuf[iBytesInOut++]=(ch>>2)&0x07;
        i++;
        pcOutBuf[iBytesInOut++]=(pcBuff[i]&0x3F)|(ch<<6);
        swap(pcOutBuf[iBytesInOut-1],pcOutBuf[iBytesInOut-2]);
    }
    else if(!(ch&0x10)) {
        i++;

        pcOutBuf[iBytesInOut++]=(ch<<4)|((pcBuff[i]>>2)&0x0F);
        ch=pcBuff[i];
        i++;
        pcOutBuf[iBytesInOut++]=(pcBuff[i]&0x3F)|(ch<<6);
        swap(pcOutBuf[iBytesInOut-1],pcOutBuf[iBytesInOut-2]);
    }
    else {
        p=(char*)&pcBuff[i];
        pcOutBuf[iBytesInOut++]='\x3F';
        pcOutBuf[iBytesInOut++]='\0';
        if(!(ch&0x08)) i+=3;
        else if(!(ch&0x04)) i+=4;
        else if(!(ch&0x02)) i+=5;
    }

    i++;

	return i;
}


int __utf8_get_char(const char *in, int *chr)
{                                       /* 2-byte, 0x80-0x7ff */
	return DecodeUTF8(in,256,(char *)chr);
}

// there is one instance of CLHistoryDlg for every history dialog on screeen.
class CLHistoryDlg
{
	public:
		HWND hWnd;

		MCONTACT hContact;
		tstring sPath;

		HWND hFindDlg;
		FINDREPLACE fr;
		TCHAR acFindStr[100];

		bool bFirstLoad;
		bool bUtf8File;

		CLHistoryDlg( MCONTACT hContact) : hContact( hContact )
		{
			hFindDlg = NULL;
			acFindStr[0] = 0;
			ZeroMemory(&fr, sizeof( fr ));
			fr.lStructSize = sizeof( fr);
			fr.hInstance = hInstance;
			fr.Flags = FR_NOUPDOWN|FR_HIDEUPDOWN;//|FR_MATCHCASE|FR_WHOLEWORD;
						     // FR_DOWN|FR_FINDNEXT|FR_NOMATCHCASE;
			fr.lpstrFindWhat = acFindStr;
			fr.wFindWhatLen = sizeof(acFindStr);
			bFirstLoad = true;
			bUtf8File = false;
		}
};

// List of all open history windows
list< CLHistoryDlg* > clHistoryDlgList;
// CRITICAL_SECTION used to access the window list
// this is nesery because UpdateFileViews is called from callback thread.
CRITICAL_SECTION csHistoryList;


// CLStreamRTFInfo is used when reading RTF into rich edit from a stream.
// RTF is used when Syntax highlighting is used.
class CLStreamRTFInfo
{
	private:
		HANDLE hFile;
		bool bHeaderWriten;
		bool bTailWriten;
		bool bCheckFirstForNick;
		bool bLastColorMyNick;

		// buffer size supplyed on win XP 4092 byte when streamin in
		// optimal size it to fully use this buffer but we can guess
		// how may bytes need converting in the file we are reading.
		BYTE abBuf[3300];
		char szMyNick[100];
		int nNickLen;
		static int nOptimalReadLen;

		int nWriteHeader( char *pszTarget, int nLen);
	public:
		bool bUtf8File;
		CLStreamRTFInfo( HANDLE hFile )
		{
			this->hFile = hFile;
			bHeaderWriten = false;
			bTailWriten = false;
			bCheckFirstForNick = false;
			bLastColorMyNick = false;
			bUtf8File = false;
			nNickLen = 0;
		}
		int nLoadFileStream(LPBYTE pbBuff, LONG cb);
};
int CLStreamRTFInfo::nOptimalReadLen = 3300;

/////////////////////////////////////////////////////////////////////
// Member Function : nWriteHeader
// Type            : Private / Public / Protected
// Parameters      : pszTarget - ?
//                   nLen      - ?
// Returns         : int
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 030204, 04 February 2003
// Developer       : KN
/////////////////////////////////////////////////////////////////////

int CLStreamRTFInfo::nWriteHeader( char *pszTarget, int nLen )
{
	COLORREF cMyText = db_get_dw(NULL,"SRMsg","Font3Col",RGB(64,0,128));
	COLORREF cYourText = db_get_dw(NULL,"SRMsg","Font0Col",RGB(240,0,0));

/* original header !!
			"{\\rtf1\\ansi\\deff0{\\fonttbl{\\f0\\fnil\\fcharset0 Courier New;}}\r\n"
			"{\\colortbl ;\\red%d\\green%d\\blue%d;\\red%d\\green%d\\blue%d;}\r\n"
			"\\viewkind4\\uc1\\pard\\cf2\\lang1033\\f0\\fs16 ",

*/
	char szRtfHeader[400];
	int nSrcLen = mir_snprintf(szRtfHeader, SIZEOF(szRtfHeader),
			"{\\rtf1\\ansi\r\n"
			"{\\colortbl ;\\red%d\\green%d\\blue%d;\\red%d\\green%d\\blue%d;}\r\n"
			"\\viewkind4\\uc1\\pard\\cf2 ",
				GetRValue(cMyText), GetGValue(cMyText), GetBValue(cMyText),
				GetRValue(cYourText), GetGValue(cYourText), GetBValue(cYourText));

	if (nSrcLen > nLen )
	{
		MessageBox(NULL, TranslateT("Failed to write to the RichEdit the buffer was to small."), MSG_BOX_TITEL, MB_OK);
		return 0; // target buffer to small
	}

	memcpy( pszTarget, szRtfHeader, nSrcLen);
	bHeaderWriten = true;
	return nSrcLen;
}

const char szNewLine[] = "\n\\par ";
const char szRtfEnd[] = "\r\n\\par }\r\n\0";

/////////////////////////////////////////////////////////////////////
// Member Function : nLoadFileStream
// Type            : Private / Public / Protected
// Parameters      : pbBuff - ?
//                   cb     - ?
// Returns         : int
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 030204, 04 February 2003
// Developer       : KN
/////////////////////////////////////////////////////////////////////

int CLStreamRTFInfo::nLoadFileStream(LPBYTE pbBuff, LONG cb)
{
	if (bTailWriten )
		return 0;

	if (nOptimalReadLen < 500 )
	{
		MessageBox(NULL, TranslateT("Error: Optimal buffer size decreased to a too low size!"), MSG_BOX_TITEL, MB_OK);
		return 0;
	}

	DWORD dwRead;
	DWORD dwToRead = nOptimalReadLen;

	if ( !ReadFile(hFile, abBuf, dwToRead, &dwRead, (LPOVERLAPPED)NULL))
		return 0;

	DWORD dwCurrent = 0;
	DWORD n = 0;
	if ( !bHeaderWriten )
	{
		if (dwRead >= 3 )
		{
			bUtf8File = bIsUtf8Header( abBuf);
			if (bUtf8File )
				n = 3;
		}
		dwCurrent += nWriteHeader( (char*)pbBuff, cb);

		tstring sMyNick = NickFromHandle(0);

		nNickLen = WideCharToMultiByte(bUtf8File ? CP_UTF8 : CP_ACP, 0, sMyNick.c_str(), (int)sMyNick.length(), szMyNick, sizeof( szMyNick ), NULL, NULL);
	}
	else
	{
		if (bCheckFirstForNick )
		{
			// Test against "<<" also
			if (( (memcmp( abBuf, szMyNick, nNickLen) == 0) ||
				   (abBuf[0] == '<' && abBuf[1] == '<')
				) != bLastColorMyNick )
			{
				// we shut only get here if we need to change color !!
				bLastColorMyNick = !bLastColorMyNick;
				// change color
				memcpy(&pbBuff[dwCurrent], bLastColorMyNick ? "\\cf1 " : "\\cf2 ", 5);
			}
			bCheckFirstForNick = false;
		}
	}

	bool bIsFileEnd = dwRead < dwToRead;

	for (; n < dwRead ; n++ )
	{
		// worst case is a file ending with \n or a unicode letter. resulting in a big unicode string
		// here we need szNewLine and szRtfEnd. the 10 is a small safty margin.
		if (dwCurrent + (sizeof( szNewLine) + sizeof( szRtfEnd) + 10) > (DWORD)cb )
		{
			// oh no !!! we have almost reached the end of the windows supplyed buffer
			// we are writing to. we need to abort mision *S*!!
			// and rewinde file
			// we will adjust the optima buffer size
			nOptimalReadLen -= 50;
			SetFilePointer( hFile, n - dwRead, NULL, FILE_CURRENT);
			return dwCurrent;
		}

		if (abBuf[n] == '\n' )
		{
			memcpy(&pbBuff[dwCurrent], szNewLine, sizeof( szNewLine )-1);
			dwCurrent += sizeof( szNewLine )-1;

			if (n + 1 >= dwRead )
			{
				// this is an anoing case because here we have read \n as the last char in the file
				// this means that the if the next data read from file begins with <UserNick> it has
				// to be highlighted
				if (!bIsFileEnd )
					bCheckFirstForNick = true;
				break;
			}

			if (abBuf[n+1] == ' ' || abBuf[n+1] == '\t' || abBuf[n+1] == '\r' )
				continue;

			if (n + nNickLen >= dwRead )
			{
				if (!bIsFileEnd )
				{
					// here we have a problem we haven't read this data yet
					// the data we need to compare to is still in the file.
					// we can't read more data from the file because the we
					// might just move the problem. if file contains \n\n\n\n\n ...

					LONG lExtraRead = (n+1) - dwRead;
					if (lExtraRead >= 0 )
						MessageBox(NULL, TranslateT("Internal error! (lExtraRead >= 0)"), MSG_BOX_TITEL, MB_OK);
					SetFilePointer( hFile, lExtraRead, NULL, FILE_CURRENT);
					bCheckFirstForNick = true;
					return dwCurrent;
				}

				if ( !bLastColorMyNick )
					continue;
				// else the last color user was my nick
				// we needd to change color to the other user color.


				/* old code !!
				DWORD dwAddedToBuf;
				if ( !ReadFile(hFile, &abBuf[dwRead], dwNeeded, &dwAddedToBuf, (LPOVERLAPPED)NULL))
					return 0;
				dwToRead += dwNeeded;
				dwRead += dwAddedToBuf;*/
			}
			else
			{
				// the data we need is here just compare
				if (( ( memcmp(&abBuf[n+1], szMyNick, nNickLen) == 0) ||
					   ( abBuf[n+1] == '<' && abBuf[n+2] == '<')
					) == bLastColorMyNick )
					continue;
			}
			// we shut only get here if we need to change color !!
			bLastColorMyNick = !bLastColorMyNick;

			// change color
			memcpy(&pbBuff[dwCurrent], bLastColorMyNick ? "\\cf1 " : "\\cf2 ", 5);
			dwCurrent += 5;
			continue;
		}
		else if (abBuf[n] == '\\' || abBuf[n] == '}' || abBuf[n] == '{')
		{
			pbBuff[dwCurrent++]='\\';
		}
		else if (bUtf8File && (abBuf[n] & 0x80))
		{
			int nValue;
			int nLen = __utf8_get_char( (const char *)&abBuf[n], &nValue);
			if(nLen+n>dwRead) {
				SetFilePointer(hFile,n-dwRead,NULL,FILE_CURRENT);
				break;
			}
			dwCurrent += sprintf( (char*)&pbBuff[dwCurrent], "\\u%d?", nValue); //!!!!!!!!!
			//continue;
/*			// Then we have an extended char in the UTF8 file.
			// we need to convert this to UCS-2 and then to \uN in the RTF
			int nUtf8Len = 1;
			while( ( (n + nUtf8Len) < dwRead) && ((abBuf[ n + nUtf8Len ] & 0xC0) == 0x80))
				nUtf8Len++;
			wchar_t szWstr[2];
			if (MultiByteToWideChar( CP_UTF8, 0, (char*)&abBuf[n], nUtf8Len, szWstr, 2) == 1 )
			{
				if ((int)(szWstr[0]) != nValue )
					__utf8_get_char( (const char *)&abBuf[n], &nValue);

//				dwCurrent += sprintf( (char*)&pbBuff[dwCurrent], "\\u%d?", (int)(szWstr[0]));
//				n += nUtf8Len - 1;
//				continue;
			}*/
			n += nLen-1;
			continue;
		}
		pbBuff[dwCurrent++] = abBuf[n];
	}

	if (bIsFileEnd )
	{// write end
		memcpy(&pbBuff[dwCurrent], szRtfEnd, sizeof( szRtfEnd )-1);
		dwCurrent += sizeof( szRtfEnd )-1;
		bTailWriten = true;
	}
	//memcpy( pbBuff, abBuf, dwRead);
	return dwCurrent;
}


/////////////////////////////////////////////////////////////////////
// Member Function : Initialize
// Type            : Global
// Parameters      : None
// Returns         : void
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 021213, 13 December 2002
// Developer       : KN
/////////////////////////////////////////////////////////////////////

void Initialize()
{
	InitializeCriticalSection(&csHistoryList);
}

/////////////////////////////////////////////////////////////////////
// Member Function : Uninitilize
// Type            : Global
// Parameters      : None
// Returns         : void
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 021213, 13 December 2002
// Developer       : KN
/////////////////////////////////////////////////////////////////////

void Uninitilize()
{
	DeleteCriticalSection(&csHistoryList);
}

/////////////////////////////////////////////////////////////////////
// Member Function : UpdateFileViews
// Type            : Global
// Parameters      : pszFile - File which has been updated
// Returns         : void
// Description     : Send a message to alle to windows that need updating
//
// References      : -
// Remarks         : -
// Created         : 021213, 13 December 2002
// Developer       : KN
/////////////////////////////////////////////////////////////////////

void UpdateFileViews( const TCHAR *pszFile )
{
	EnterCriticalSection(&csHistoryList);

	list< CLHistoryDlg* >::const_iterator iterator;
	for (iterator = clHistoryDlgList.begin() ; iterator != clHistoryDlgList.end() ; ++iterator )
	{
		CLHistoryDlg* pcl = (*iterator);
		if (pcl->sPath == pszFile )
		{
			PostMessage( pcl->hWnd, WM_RELOAD_FILE, 0, 0);
		}
	}
	LeaveCriticalSection(&csHistoryList);
}

/////////////////////////////////////////////////////////////////////
// Member Function : bOpenExternaly
// Type            : Global
// Parameters      : hContact - ?
// Returns         : Returns true if
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 021010, 10 October 2002
// Developer       : KN
/////////////////////////////////////////////////////////////////////

bool bOpenExternaly( MCONTACT hContact )
{
	tstring sPath = GetFilePathFromUser( hContact);

	if (sFileViewerPrg.empty())
	{
		SHELLEXECUTEINFO st = {0};
		st.cbSize = sizeof(st);
		st.fMask = SEE_MASK_INVOKEIDLIST;
		st.hwnd = NULL;
		st.lpFile = sPath.c_str();
		st.nShow = SW_SHOWDEFAULT;
		ShellExecuteEx(&st);
		return true;
	}
	tstring sTmp = sFileViewerPrg;
	sTmp += _T(" \"");
	sTmp += sPath;
	sTmp += '\"';

	STARTUPINFO sStartupInfo = { 0 };
	GetStartupInfo(&sStartupInfo); // we parse oure owne info on
	sStartupInfo.lpTitle = (TCHAR*)sFileViewerPrg.c_str();
	PROCESS_INFORMATION stProcesses = {0};

	if ( !CreateProcess(NULL,
				(TCHAR*)sTmp.c_str(),
				NULL,
				NULL,
				FALSE,
				CREATE_DEFAULT_ERROR_MODE | CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP,
				NULL,
				NULL,
				&sStartupInfo,
				&stProcesses))
	{
		DisplayLastError( LPGENT("Failed to execute external file view"));
	}
	return true;
}


/////////////////////////////////////////////////////////////////////
// Member Function : bGetInternalViewer
// Type            : Global
// Parameters      : None
// Returns         : Returns true if
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 021016, 16 October 2002
// Developer       : KN
/////////////////////////////////////////////////////////////////////

bool bUseInternalViewer()
{
	return bUseIntViewer;
}

/////////////////////////////////////////////////////////////////////
// Member Function : bUseInternalViewer
// Type            : Global
// Parameters      : bNew - ?
// Returns         : Returns true if
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 021016, 16 October 2002
// Developer       : KN
/////////////////////////////////////////////////////////////////////

bool bUseInternalViewer( bool bNew )
{
	bUseIntViewer = bNew;
   if (bUseIntViewer && !hRichEditDll )
   {
      hRichEditDll = LoadLibraryA("RICHED32.DLL");
		if (!hRichEditDll )
		{
			DisplayLastError( LPGENT("Failed to load Rich Edit ( RICHED32.DLL )" ));
			return false;
		}
   }
	return true;
}


/////////////////////////////////////////////////////////////////////
// Member Function : RichEditStreamLoadFile
// Type            : Global
// Parameters      : dwCookie - ?
//                   pbBuff   - ?
//                   cb       - ?
//                   pcb      - ?
// Returns         : DWORD CALLBACK
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 021010, 10 October 2002
// Developer       : KN
/////////////////////////////////////////////////////////////////////

DWORD CALLBACK RichEditStreamLoadFile(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
   ReadFile((HANDLE)dwCookie, pbBuff, (DWORD)cb, (DWORD *)pcb, 	(LPOVERLAPPED)NULL);
	return (DWORD) ( *pcb >= 0 ? NOERROR : ( *pcb = 0, E_FAIL));
}

DWORD CALLBACK RichEditRTFStreamLoadFile(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	*pcb = ((CLStreamRTFInfo *)dwCookie)->nLoadFileStream(pbBuff, cb);
	if (*pcb )
		return NOERROR;
	return (DWORD)E_FAIL;
}

DWORD CALLBACK RichEditStreamSaveFile(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	WriteFile((HANDLE)dwCookie, pbBuff, cb, (DWORD*)pcb, (LPOVERLAPPED)NULL);
	return *pcb != cb;
}

/*
DWORD dwCurPos = 0;
DWORD dwDataRead = 0;
BYTE * pabFileData = NULL;

DWORD CALLBACK RichEditStreamLoadFile(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	*pcb = 0;
	while( dwCurPos < dwDataRead && *pcb < cb )
	{
		pbBuff[ *pcb ] = pabFileData[ dwCurPos ];
		dwCurPos++;
		(*pcb)++;
	}
	return (DWORD) ( *pcb >= 0 ? NOERROR : ( *pcb = 0, E_FAIL));
}
*/
/////////////////////////////////////////////////////////////////////
// Member Function : bLoadFile
// Type            : Global
// Parameters      : hwndDlg  - ?
//                   hContact - ?
// Returns         : Returns true if
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 021010, 10 October 2002
// Developer       : KN
/////////////////////////////////////////////////////////////////////

bool bLoadFile( HWND hwndDlg, CLHistoryDlg * pclDlg )
{
	DWORD dwStart = GetTickCount();

	HWND hRichEdit = GetDlgItem(hwndDlg, IDC_RICHEDIT);
	if (!hRichEdit) {
		MessageBox(hwndDlg, TranslateT("Failed to get handle to RichEdit!"), MSG_BOX_TITEL, MB_OK);
		return false;
	}

	HANDLE hFile = CreateFile( pclDlg->sPath.c_str(), GENERIC_READ, 
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		int nDBCount = db_event_count(pclDlg->hContact);
		TCHAR szTmp[1500];

		if (nDBCount == -1 )
			mir_sntprintf(szTmp, 1499, TranslateT("Failed to open file\r\n%s\r\n\r\nContact handle is invalid"), pclDlg->sPath.c_str());
		else
			mir_sntprintf(szTmp, 1499, TranslateT("Failed to open file\r\n%s\r\n\r\nMiranda database contains %d events"), pclDlg->sPath.c_str(), nDBCount);

		SETTEXTEX stText = {0};
		stText.codepage =	1200;
		SendMessage(hRichEdit, EM_SETTEXTEX, (WPARAM) &stText, (LPARAM)szTmp);
		return false;
	}

	POINT ptOldPos;
	SendMessage( hRichEdit, EM_GETSCROLLPOS, 0, (LPARAM)&ptOldPos);

	bool bScrollToBottom = true;
	if (pclDlg->bFirstLoad )
		pclDlg->bFirstLoad = false;
	else {
		SCROLLINFO sScrollInfo = { 0 };
		sScrollInfo.cbSize = sizeof( SCROLLINFO);
		sScrollInfo.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
		if (GetScrollInfo( hRichEdit,SB_VERT,&sScrollInfo))
			bScrollToBottom = sScrollInfo.nPos + (int)sScrollInfo.nPage + 50 > sScrollInfo.nMax;
	}

	HMENU hSysMenu = GetSystemMenu(hwndDlg, FALSE);
	bool bUseSyntaxHL = (GetMenuState( hSysMenu , ID_FV_SYNTAX_HL, MF_BYCOMMAND) & MF_CHECKED)!=0;

	EDITSTREAM eds;
	eds.dwError = 0;

	if (bUseSyntaxHL) {
		SendMessage(hRichEdit, EM_EXLIMITTEXT, 0, 0x7FFFFFFF);

		CLStreamRTFInfo clInfo( hFile);
		eds.dwCookie = (DWORD)&clInfo;
		eds.pfnCallback = RichEditRTFStreamLoadFile;

		SendMessage(hRichEdit, EM_STREAMIN, (WPARAM)SF_RTF, (LPARAM)&eds);
		pclDlg->bUtf8File = clInfo.bUtf8File;
	}
	else {
		eds.dwCookie = (DWORD)hFile;
		eds.pfnCallback = RichEditStreamLoadFile;

		SendMessage(hRichEdit, EM_STREAMIN, (WPARAM)SF_TEXT, (LPARAM)&eds);
	}
	CloseHandle(hFile);

	TCHAR szTmp[100];
	mir_sntprintf(szTmp, 99, _T("File open time %d\n"), GetTickCount() - dwStart);
	OutputDebugString(szTmp);

	GETTEXTLENGTHEX sData = { 0 };
	sData.flags = GTL_NUMCHARS;
	sData.flags = GTL_DEFAULT;

	DWORD dwDataRead = (DWORD)SendMessage( hRichEdit, EM_GETTEXTLENGTHEX, (WPARAM)&sData, 0);
	SendMessage(hRichEdit, EM_SETSEL, dwDataRead - 1, dwDataRead - 1);

	if ( !bScrollToBottom )
		SendMessage(hRichEdit, EM_SETSCROLLPOS, 0, (LPARAM)&ptOldPos);

	mir_sntprintf(szTmp, 99, TranslateT("With scroll to bottom %d\n"), GetTickCount() - dwStart);
	OutputDebugString(szTmp);
	return true;
}

/////////////////////////////////////////////////////////////////////
// Member Function : bAdvancedCopy
// Type            : Global
// Parameters      : hwnd - handle to RichEdit control
// Returns         : Returns true if text was copied to the clipboard
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 030730, 30 juli 2003
// Developer       : KN
/////////////////////////////////////////////////////////////////////

bool bAdvancedCopy(HWND hwnd)
{
	CHARRANGE sSelectRange;
	SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&sSelectRange);
	int nSelLenght = sSelectRange.cpMax - sSelectRange.cpMin + 1; // +1 for null termination
	if (nSelLenght > 1 )
	{
		OpenClipboard(NULL);
		EmptyClipboard();

		TCHAR *pszSrcBuf = new TCHAR[ nSelLenght];
		pszSrcBuf[0] = 0;
		SendMessage(hwnd, EM_GETSELTEXT, 0, (LPARAM)pszSrcBuf);

		HANDLE hDecMem = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, nSelLenght);
		TCHAR *pszCurDec = (TCHAR*)GlobalLock(hDecMem);

		bool bInSpaces = false;
		for (TCHAR *pszCur = pszSrcBuf ; pszCur[0] ; pszCur++) {
			if (bInSpaces) {
				if (pszCur[0] == ' ' )
					continue;
				bInSpaces = false;
			}

			if (pszCur[0] == '\n' )
				bInSpaces = true;

			pszCurDec[0] = pszCur[0];
			pszCurDec++;
		}
		pszCurDec[0] = 0;
		GlobalUnlock(hDecMem);

		SetClipboardData(CF_TEXT,hDecMem);
		delete [] pszSrcBuf;
		CloseClipboard();
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////
// Member Function : EditSubclassProc
// Type            : Global
// Parameters      : hwnd   - ?
//                   uMsg   - ?
//                   wParam - ?
//                   lParam - ?
// Returns         : LRESULT CALLBACK
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 021013, 13 October 2002
// Developer       : KN
/////////////////////////////////////////////////////////////////////

LRESULT CALLBACK EditSubclassProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CLHistoryDlg *pclDlg = (CLHistoryDlg*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	switch(msg) {
	case WM_CONTEXTMENU:
		{
			HMENU nMenu = LoadMenu(hInstance, MAKEINTRESOURCE( IDR_FV_EDIT));
			HMENU nSubMenu = GetSubMenu(nMenu, 0);
			POINT pt = { (short)LOWORD(lParam), (short)HIWORD(lParam) };

			if(pt.x == -1 && pt.y == -1) {
				DWORD dwStart,dwEnd;
				SendMessage(  hwnd, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
				SendMessage(  hwnd, EM_POSFROMCHAR, (WPARAM)&pt, (LPARAM)dwEnd);
				ClientToScreen(hwnd, &pt);
			}
			TrackPopupMenu( nSubMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, 0);

			DestroyMenu( nSubMenu);
			DestroyMenu( nMenu);
			return TRUE;
		}
	case WM_GETDLGCODE:
		return DLGC_WANTARROWS;

	case WM_COPY:
		// not working for "CTRL + C"
		if (bAdvancedCopy(hwnd))
			return TRUE;
		break;

	case WM_KEYDOWN:
		if ((wParam == VK_INSERT || wParam == 'C') && (GetKeyState(VK_CONTROL) & 0x80))
			if (bAdvancedCopy(hwnd))
				return TRUE;
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_EDIT_COPY:
			SendMessage(hwnd, WM_COPY, 0, 0);
			return TRUE;
		}
	}

	if (msg == UM_FIND_CMD) {
		FINDREPLACE *fr = (FINDREPLACE*)lParam;
		if (fr->Flags & FR_DIALOGTERM ) {
			pclDlg->hFindDlg = NULL;
			return 0;
		}

		if (fr->Flags & FR_FINDNEXT) {
			FINDTEXT ft = { 0 };
			ft.lpstrText = fr->lpstrFindWhat;

			SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&ft.chrg);
			ft.chrg.cpMin = ft.chrg.cpMax+1;
			ft.chrg.cpMax = -1;
			LRESULT res = SendMessage(hwnd, EM_FINDTEXTW, (WPARAM)fr->Flags,(LPARAM)&ft);
			if(res == -1) {
				ft.chrg.cpMin = 0;
				res = (int)SendMessage(hwnd, EM_FINDTEXTW, (WPARAM)fr->Flags,(LPARAM)&ft);
				if(res == -1) {
					MessageBox(hwnd, TranslateT("Search string was not found!"), MSG_BOX_TITEL, MB_OK);
					return 0;
				}
			}
			ft.chrg.cpMin = LONG(res);
			ft.chrg.cpMax = LONG(res + _tcslen(fr->lpstrFindWhat));
			SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&ft.chrg);
			return 0;
		}
	}
	return mir_callNextSubclass(hwnd, EditSubclassProc, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////
// Member Function : SetWindowsCtrls
// Type            : Global
// Parameters      : hwndDlg - ?
// Returns         : void
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 021001, 01 October 2002
// Developer       : KN
/////////////////////////////////////////////////////////////////////

void SetWindowsCtrls( HWND hwndDlg )
{
	RECT rNewSize;
	GetClientRect(hwndDlg, &rNewSize);

	const int nSpacing = 12;
	HWND hButton = GetDlgItem(hwndDlg, IDOK);

	RECT rCurSize;
	GetWindowRect(hButton, &rCurSize);
	int nButtonHeight = rCurSize.bottom - rCurSize.top;

	SetWindowPos(GetDlgItem(hwndDlg, IDC_RICHEDIT ), 0,
		nSpacing, nSpacing,
		rNewSize.right - (nSpacing * 2),
		rNewSize.bottom - ( nSpacing * 3 + nButtonHeight ),
		SWP_NOZORDER);

	int nButtonWidth = rCurSize.right - rCurSize.left;
	int nButtonSpace = (rNewSize.right - ( 3 * nButtonWidth )) / 4;
	int nButtonTop = rNewSize.bottom - ( nSpacing + nButtonHeight);
	int nCurLeft = nButtonSpace;

	SetWindowPos(GetDlgItem(hwndDlg, IDC_FV_FIND ), 0,
		nCurLeft, nButtonTop, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

	nCurLeft += nButtonSpace + nButtonWidth;

	SetWindowPos(GetDlgItem(hwndDlg, IDC_FV_EXTERNAL ), 0,
		nCurLeft, nButtonTop, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

	nCurLeft += nButtonSpace + nButtonWidth;

	SetWindowPos(hButton, 0, nCurLeft, nButtonTop, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

}

/////////////////////////////////////////////////////////////////////
// Member Function : SetRichEditFont
// Type            : Global
// Parameters      : hRichEdit    - RichEdit to set the font in
//                   bUseSyntaxHL - Is Syntax hilighting is used the color
//												will not be set
// Returns         : void
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 030205, 05 February 2003
// Developer       : KN
/////////////////////////////////////////////////////////////////////

void SetRichEditFont(HWND hRichEdit, bool bUseSyntaxHL )
{
	CHARFORMAT ncf = { 0 };
	ncf.cbSize = sizeof(CHARFORMAT);
	ncf.dwMask = CFM_BOLD | CFM_FACE | CFM_ITALIC | CFM_SIZE | CFM_UNDERLINE;
	ncf.dwEffects = db_get_dw(NULL, MODULE, szFileViewDB "TEffects", 0);
	ncf.yHeight = db_get_dw(NULL, MODULE, szFileViewDB "THeight", 165);
	_tcscpy(ncf.szFaceName , _DBGetString(NULL, MODULE, szFileViewDB "TFace", _T("Courier New")).c_str());

	if (!bUseSyntaxHL) {
		ncf.dwMask |= CFM_COLOR;
		ncf.crTextColor = db_get_dw(NULL, MODULE, szFileViewDB "TColor", 0);
	}
	SendMessage(hRichEdit, EM_SETCHARFORMAT, (WPARAM)SCF_ALL, (LPARAM)&ncf);

}

/////////////////////////////////////////////////////////////////////
// Member Function : DlgProcFileViewer
// Type            : Global
// Parameters      : hwndDlg - ?
//                   msg     - ?
//                   wParam  - ?
//                   lParam  - ?
// Returns         : static BOOL CALLBACK
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 020929, 29 September 2002
// Developer       : KN
/////////////////////////////////////////////////////////////////////

static INT_PTR CALLBACK DlgProcFileViewer(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CLHistoryDlg * pclDlg = (CLHistoryDlg *)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		pclDlg = (CLHistoryDlg *)lParam;
		
		SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(hInstance, MAKEINTRESOURCE(IDI_EXPORT_MESSAGE)));
		{	
			HWND hRichEdit = GetDlgItem(hwndDlg, IDC_RICHEDIT);
			mir_subclassWindow(hRichEdit, EditSubclassProc);
			SetWindowLongPtr(hRichEdit, GWLP_USERDATA, (LONG_PTR)pclDlg);
			SendMessage(hRichEdit, EM_SETEVENTMASK, 0, ENM_LINK);
			SendMessage(hRichEdit, EM_AUTOURLDETECT, TRUE, 0);

			HMENU hSysMenu = GetSystemMenu(hwndDlg, FALSE);
			InsertMenu(hSysMenu, 0, MF_SEPARATOR | MF_BYPOSITION, 0, 0);
			InsertMenu(hSysMenu, 0, MF_STRING | MF_BYPOSITION, ID_FV_SAVE_AS_RTF, LPGENT("Save as RTF"));
			InsertMenu(hSysMenu, 0, MF_SEPARATOR | MF_BYPOSITION, 0, 0);

			BYTE bUseCC = (BYTE)db_get_b(NULL, MODULE, szFileViewDB "UseCC", 0);
			InsertMenu(hSysMenu, 0, MF_STRING | MF_BYPOSITION | ( bUseCC ? MF_CHECKED : 0 ), ID_FV_COLOR, LPGENT("Color..."));

			if (bUseCC)
				SendMessage( hRichEdit, EM_SETBKGNDCOLOR, 0, db_get_dw(NULL, MODULE, szFileViewDB "CustomC", RGB(255,255,255)));

			InsertMenu(hSysMenu, 0, MF_STRING | MF_BYPOSITION, ID_FV_FONT, LPGENT("Font..."));

			bool bUseSyntaxHL = db_get_b(NULL, MODULE, szFileViewDB "UseSyntaxHL", 1) != 0;
			InsertMenu(hSysMenu, 0, MF_STRING | MF_BYPOSITION | ( bUseSyntaxHL ? MF_CHECKED : 0 ), ID_FV_SYNTAX_HL, LPGENT("Syntax highlight"));

			SetRichEditFont(hRichEdit, bUseSyntaxHL);

			TranslateDialogDefault(hwndDlg);

			Utils_RestoreWindowPosition(hwndDlg,pclDlg->hContact,MODULE,szFileViewDB);

			pclDlg->sPath = GetFilePathFromUser( pclDlg->hContact);

			SetWindowsCtrls(hwndDlg);

			bLoadFile(hwndDlg, pclDlg);

			TCHAR szFormat[200];
			TCHAR szTitle[200];
			if (GetWindowText(hwndDlg, szFormat, sizeof( szFormat))) {
				const TCHAR *pszNick = NickFromHandle( pclDlg->hContact);
				tstring sPath = pclDlg->sPath;
				string::size_type n = sPath.find_last_of( '\\');
				if (n != sPath.npos )
					sPath.erase( 0, n + 1);

				if (mir_sntprintf(szTitle, SIZEOF(szTitle), szFormat, pszNick, sPath.c_str(), (pclDlg->bUtf8File ? _T("UTF8"):_T("ANSI"))) > 0)
					SetWindowText(hwndDlg, szTitle);
			}

			WindowList_Add(hInternalWindowList,hwndDlg,pclDlg->hContact);
		}
		return TRUE;

	case WM_RELOAD_FILE:
		bLoadFile(hwndDlg, pclDlg);
		return TRUE;

	case WM_SIZE:
	case WM_SIZING:
		SetWindowsCtrls(hwndDlg);
		return TRUE;

	case WM_NCDESTROY:
		EnterCriticalSection(&csHistoryList);
		clHistoryDlgList.remove( pclDlg);
		LeaveCriticalSection(&csHistoryList);

		delete pclDlg;
		SetWindowLongPtr(hwndDlg,GWLP_USERDATA,0);
		return 0;

	case WM_DESTROY:
		Utils_SaveWindowPosition(hwndDlg,pclDlg->hContact,MODULE,szFileViewDB);
		WindowList_Remove(hInternalWindowList,hwndDlg);
		return 0;

	case WM_SYSCOMMAND:
		{
			HMENU hSysMenu = GetSystemMenu(hwndDlg, FALSE);
			bool bUseSyntaxHL = (GetMenuState( hSysMenu , ID_FV_SYNTAX_HL, MF_BYCOMMAND) & MF_CHECKED)!=0;
			HWND hRichEdit = GetDlgItem(hwndDlg, IDC_RICHEDIT);

			if ((wParam & 0xFFF0) == ID_FV_FONT) {
				LOGFONT lf = { 0 };
				lf.lfHeight = 14L;

				DWORD dwEffects = db_get_dw(NULL, MODULE, szFileViewDB "TEffects", 0);
				lf.lfWeight = (dwEffects & CFE_BOLD) ? FW_BOLD : 0;
				lf.lfUnderline = (dwEffects & CFE_UNDERLINE) != 0;
				lf.lfStrikeOut = (dwEffects & CFE_STRIKEOUT) != 0;
				lf.lfItalic = (dwEffects & CFE_ITALIC) != 0;

				_tcscpy(lf.lfFaceName, _DBGetString(NULL, MODULE, szFileViewDB "TFace", _T("Courier New")).c_str());
				CHOOSEFONT cf = { 0 };
				cf.lStructSize = sizeof( cf);
				cf.hwndOwner = hwndDlg;
				cf.lpLogFont = &lf;
				cf.rgbColors = db_get_dw(NULL, MODULE, szFileViewDB "TColor", 0);
				cf.Flags = CF_EFFECTS | CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;

				if ( ChooseFont(&cf)) {
					DWORD dwEffects = (lf.lfWeight == FW_BOLD ?  CFE_BOLD : 0) |
						(lf.lfItalic          ?  CFE_ITALIC : 0) |
						(lf.lfStrikeOut 	   ? CFE_STRIKEOUT : 0) |
						(lf.lfUnderline    ?  CFE_UNDERLINE : 0);

					db_set_dw(NULL, MODULE, szFileViewDB "TEffects", dwEffects);
					db_set_dw(NULL, MODULE, szFileViewDB "THeight", cf.iPointSize * 2);
					db_set_dw(NULL, MODULE, szFileViewDB "TColor", cf.rgbColors);
					db_set_ts(NULL, MODULE, szFileViewDB "TFace", lf.lfFaceName);
					SetRichEditFont( hRichEdit, bUseSyntaxHL);
				}
				return TRUE;
			}
			if ((wParam & 0xFFF0) == ID_FV_COLOR) {
				BYTE bUseCC = ! db_get_b(NULL, MODULE, szFileViewDB "UseCC", 0);
				if (bUseCC) {
					CHOOSECOLOR cc = {0};
					cc.lStructSize = sizeof( cc);
					cc.hwndOwner = hwndDlg;
					cc.rgbResult = db_get_dw(NULL, MODULE, szFileViewDB "CustomC", RGB(255,255,255));
					cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
					static COLORREF MyCustColors[16] = { 0xFFFFFFFF };
					cc.lpCustColors = MyCustColors;
					if ( ChooseColor(&cc)) {
						SendMessage( hRichEdit, EM_SETBKGNDCOLOR, 0, cc.rgbResult);
						db_set_dw(NULL, MODULE, szFileViewDB "CustomC", cc.rgbResult);
					}
					else {
						CommDlgExtendedError();
						return TRUE;
					}
				}
				else SendMessage( hRichEdit, EM_SETBKGNDCOLOR, TRUE, 0);

				CheckMenuItem( hSysMenu, ID_FV_COLOR, MF_BYCOMMAND | (bUseCC ? MF_CHECKED : 0));
				db_set_b(NULL, MODULE, szFileViewDB "UseCC", bUseCC);
				return TRUE;
			}
			if ((wParam & 0xFFF0) == ID_FV_SYNTAX_HL) {
				// we use the current state from the menu not the DB value
				// because we want to toggel the option for this window
				// still the new option selected will be stored.
				// so user may open 2 windows, now he can set SyntaxHL in both.

				bUseSyntaxHL = !bUseSyntaxHL;
				CheckMenuItem( hSysMenu, ID_FV_SYNTAX_HL, MF_BYCOMMAND | (bUseSyntaxHL ? MF_CHECKED : 0));
				db_set_b(NULL, MODULE, szFileViewDB "UseSyntaxHL", bUseSyntaxHL);

				if (bUseSyntaxHL )
					bLoadFile(hwndDlg, pclDlg);
				else
					SetRichEditFont( hRichEdit, bUseSyntaxHL);

				return TRUE;
			}
			if ((wParam & 0xFFF0) == ID_FV_SAVE_AS_RTF) {
				tstring sFile = pclDlg->sPath;
				sFile += _T(".rtf");
				HANDLE hFile = CreateFile( sFile.c_str(), GENERIC_WRITE,
					FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

				if (hFile == INVALID_HANDLE_VALUE) {
					DisplayLastError( LPGENT("Failed to create file"));
					return TRUE;
				}

				EDITSTREAM eds;
				eds.dwCookie = (DWORD )hFile;
				eds.dwError = 0;
				eds.pfnCallback = RichEditStreamSaveFile;
				LRESULT nWriteOk = SendMessage(hRichEdit, EM_STREAMOUT, (WPARAM)SF_RTF, (LPARAM)&eds);
				if (nWriteOk <= 0 || eds.dwError != 0) {
					DisplayLastError( TranslateT("Failed to save file"));
					CloseHandle( hFile);
					return TRUE;
				}
				CloseHandle( hFile);
				tstring sReport = TranslateT("History was saved successfully in file\r\n");
				sReport += sFile;
				MessageBox(NULL, sReport.c_str(),MSG_BOX_TITEL,MB_OK);
				return TRUE;
			}
			return FALSE;
		}

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
		case IDOK:
			DestroyWindow(hwndDlg);
			return TRUE;
		case IDC_FV_EXTERNAL:
			bOpenExternaly( pclDlg->hContact);
			return TRUE;
		case IDC_FV_FIND:
			if (pclDlg->hFindDlg ) {
				BringWindowToTop( pclDlg->hFindDlg);
				return TRUE;
			}
			pclDlg->fr.hwndOwner = GetDlgItem(hwndDlg, IDC_RICHEDIT);
			pclDlg->hFindDlg = FindText(&pclDlg->fr);
			return TRUE;
		}
		break;

	case WM_NOTIFY:
		if (((NMHDR*)lParam)->idFrom == IDC_RICHEDIT) {
			if (((NMHDR*)lParam)->code == EN_LINK) {
				ENLINK* pstLink = (ENLINK*)lParam;
				if (pstLink->msg == WM_LBUTTONUP) {
					TCHAR szUrl[ 500 ];
					if ((pstLink->chrg.cpMax - pstLink->chrg.cpMin) > (sizeof( szUrl) - 2))
						return FALSE;

					TEXTRANGE stToGet;
					stToGet.chrg = pstLink->chrg;
					stToGet.lpstrText = szUrl;
					if (SendMessage( pstLink->nmhdr.hwndFrom, EM_GETTEXTRANGE, 0, (LPARAM)&stToGet) > 0 )
						CallService(MS_UTILS_OPENURL,1,(LPARAM)szUrl);

					return TRUE;
				}
			}
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////
// Member Function : bShowFileViewer
// Type            : Global
// Parameters      : hContact - ?
// Returns         : Returns true if
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 020929, 29 September 2002
// Developer       : KN
/////////////////////////////////////////////////////////////////////

bool bShowFileViewer(MCONTACT hContact)
{
	HWND hInternalWindow = WindowList_Find(hInternalWindowList,hContact);
	if(hInternalWindow) {
		SetForegroundWindow(hInternalWindow);
		SetFocus(hInternalWindow);
		return true;
	}

	CLHistoryDlg *pcl = new CLHistoryDlg( hContact);
	pcl->hWnd = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_FILE_VIEWER), NULL, DlgProcFileViewer, (LPARAM)pcl);
	if (pcl->hWnd) {
		EnterCriticalSection(&csHistoryList);
		clHistoryDlgList.push_front(pcl);
		LeaveCriticalSection(&csHistoryList);

		ShowWindow(pcl->hWnd, SW_SHOWNORMAL);
		return true;
	}
	DisplayLastError( LPGENT("Failed to create history dialog"));
	delete pcl;
	return false;
}
