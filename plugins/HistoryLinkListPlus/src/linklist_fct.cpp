// History Linklist Plus
// Copyright (C) 2010 Thomas Wendel, gureedo
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.


#include "linklist.h"

extern HINSTANCE hInst;
extern HANDLE hWindowList;

/*
The hyperlink detection in this function is taken from the
Miranda core. It looks a bit sophisticated. I'd made a few 
changes but I didn't really understand what these guys did! 
Great job! It works! ;-)
*/
int ExtractURI(DBEVENTINFO *dbei, HANDLE hEvent, LISTELEMENT *listStart)
{
	int wordStart,i,iLastAlphaNum, linkFound=0;
	int charCount=0,cpLastAlphaNum=0,cpWordStart;
	LPCTSTR msg;
	LPTSTR word, wordsearch, date_ptr, time_ptr;
	static LPCTSTR hyperlinkPrefixes[] = {
		_T("http://"),_T("ftp://"),_T("https://"),_T("mailto:"),_T("www."),_T("ftp."),
		_T("icq#"),_T("gopher://"),_T("news://"),_T("file://"),_T("\\\\")
	};
	static LPCTSTR hyperlinkSubstrings[] = {
		_T(".com/"),_T(".net/"),_T(".org/"),_T(".co.uk"),_T(".ru")
	};
	DBTIMETOSTRINGT dbtimestring;
	LISTELEMENT *newElement, *actualElement;
	TCHAR templink[LINK_MAX+1];
	TCHAR dbdate[DATE_SIZE + TIME_SIZE];
	BYTE type = LINK_UNKNOWN;
	int direction;
	TCHAR date[DATE_SIZE+1];
	TCHAR time[TIME_SIZE+1];
	TCHAR link[LINK_MAX+1];

	if ( listStart == NULL )
		return -1;

	memset(link, 0, sizeof(link));
	memset(date, 0, sizeof(date));
	memset(time, 0, sizeof(time));

	msg = DbGetEventTextT(dbei, CP_ACP);
	
	if ( msg == NULL )
		return 0;

	for ( i=0; msg[i]; ) 
	{
		//hyperlinks are delimited by: <non-alphanumeric>"hyperlink"<whitespace>
		//then all punctuation is stripped from the end of "hyperlink"
		iLastAlphaNum = 0;
		while ( msg[i] && !_istalnum(msg[i]))
		{
			// support for files
			if ( (msg[i]==_T('\\')) && (msg[i+1]==_T('\\')) && (_istalnum(msg[i+2])))
			{ 
				break;
			}

			if(IsDBCSLeadByte(msg[i]) && msg[i+1]) i++;

			i++;
			if ( msg[i] != _T('\n')) charCount++;
		}
		if ( msg[i] == _T('\0')) break;

		cpWordStart = charCount;
		wordStart = i;
			
		while ( msg[i] && !_istspace(msg[i])) 
		{

			if ( IsDBCSLeadByte(msg[i] ) && msg[i+1]) i++;
			else

			if ( _istalnum(msg[i]) || msg[i]==_T('/')) 
			{
				cpLastAlphaNum = charCount; 
				iLastAlphaNum = i;
			}
			charCount++;
			i++;
		}

		charCount = cpLastAlphaNum+1;
		i = iLastAlphaNum+1;

		if ( i-wordStart >= 7 ) 
		{
			int j, isLink = 0, wordlen;

			wordlen = i-wordStart+1;
			word = (LPTSTR)malloc(wordlen*sizeof(TCHAR));
			wordsearch = (LPTSTR)malloc(wordlen*sizeof(TCHAR));
			
			_tcsncpy_s(word, wordlen, msg+wordStart, wordlen-1);
			_tcsncpy_s(wordsearch, wordlen, msg+wordStart, wordlen-1);
			CharLower(wordsearch);
			
			for ( j=0; j<_countof(hyperlinkPrefixes); j++ )
			{
				if ( !_tcsncmp(wordsearch, hyperlinkPrefixes[j], _tcslen(hyperlinkPrefixes[j]))) 
				{
					isLink = 1; 
					break;
				}
			}
			
			if ( !isLink ) {
				for ( j=0; j<_countof(hyperlinkSubstrings); j++ )
				{
					if ( _tcsstr(wordsearch+1,hyperlinkSubstrings[j]))
					{
						isLink = 1; 
						break;
					}
				}
			}
		
			if ( _tcschr(wordsearch,_T('@')) && _tcschr(wordsearch,_T('.')) && !_tcschr(wordsearch,_T(':')) && !_tcschr(wordsearch,_T('/')))
			{
				isLink = 1;	//e-mail addresses
				type = LINK_MAIL;
			}
			else if ( isLink ) {
				type = LINK_URL;
			}
			
			if ( isLink && ( (i-wordStart+1) <= (int)(LINK_MAX - _mstrlen(_T("http://")))))
			{
				LPTSTR tok_ctx;

				if ( (_tcsstr(wordsearch, _T("www.")) != NULL) && (_tcsstr(wordsearch, _T("http://")) == NULL))
				{
					_tcsncpy_s(link, _T("http://"), _mstrlen(_T("http://")));
					// Link longer than defined max -> cut link to max
					if ( (i-wordStart+1) > (int)(LINK_MAX-_mstrlen(_T("http://"))))
						_tcsncpy_s(link + _mstrlen(_T("http://")), _countof(link), word, LINK_MAX - _mstrlen(_T("http://")));
					else
						_tcsncpy_s(link + _mstrlen(_T("http://")), _countof(link), word, i-wordStart+1);
				}
				else
				{
					size_t link_size;
					if ( (i-wordStart+1) > LINK_MAX )
						link_size = LINK_MAX;
					else
						link_size = i-wordStart+1;
					_tcsncpy_s(link, word, link_size);
				}

				dbtimestring.szFormat = _T("d-t");
				dbtimestring.szDest = dbdate;
				dbtimestring.cbDest = _countof(dbdate);
				CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT,(WPARAM)dbei->timestamp, (LPARAM)&dbtimestring);
				date_ptr = _tcstok_s(dbdate, _T("-"), &tok_ctx);
				time_ptr = _tcstok_s(NULL, _T("-"), &tok_ctx);
				_tcsncpy_s(date, date_ptr, _TRUNCATE);
				_tcsncpy_s(time, time_ptr, _TRUNCATE);
				// Prevent overflow
				date[DATE_SIZE] = _T('\0');
				time[TIME_SIZE] = _T('\0');

				if ( dbei->flags & DBEF_SENT ) {
					direction = DIRECTION_OUT;
				} else {
					direction = DIRECTION_IN;
				}

				if ( type == LINK_MAIL && _tcsstr(link, _T("mailto:")) == NULL )
				{
					_tcsncpy_s(templink, link, _TRUNCATE);
					_tcsncpy_s(link, _T("mailto:"), _TRUNCATE);
					_tcsncpy_s(link + _mstrlen(_T("mailto:")), _countof(link), templink, _TRUNCATE);
				}
				
				// Add new Element to list:
				newElement = (LISTELEMENT*)malloc(sizeof(LISTELEMENT));
				if ( newElement == NULL )
					return -1;

				memset(newElement, 0, sizeof(LISTELEMENT));
				newElement->direction = direction;
				newElement->type = type;
				_tcsncpy_s(newElement->date, date, _TRUNCATE);
				_tcsncpy_s(newElement->time, time, _TRUNCATE);
				_tcsncpy_s(newElement->link, link, _TRUNCATE);
				newElement->hEvent = hEvent;
				
				actualElement = listStart;
				while ( actualElement->nextElement != NULL )
				{
					actualElement = actualElement->nextElement; 
				}
				
				actualElement->nextElement = newElement;
				linkFound++;
			}
			free(word);
			free(wordsearch);
		}
	}

	mir_free((void*)msg);

	return linkFound;
}


/*
Remove the linked list an free memory
*/
int RemoveList(LISTELEMENT *listStart)
{
	LISTELEMENT *actualElement, *tempElement;

	if ( listStart == NULL )
		return -1;

	actualElement = listStart->nextElement;
	while ( actualElement != NULL )
	{
		tempElement = actualElement->nextElement;
		free(actualElement);
		actualElement = tempElement;
	}
	free(listStart);
	return 0;
}


/*
Count the elements of the list
*/
int ListCount(LISTELEMENT *listStart)
{
	LISTELEMENT *actualElement;
	int count = 0;

	if ( listStart == NULL )
		return -1;
	
	actualElement = listStart->nextElement;
	while ( actualElement != NULL )
	{
		count++;
		actualElement = actualElement->nextElement;
	}
	return count;
}


/*
Fill the richedit field with informations ;-)

Special thanks to MatriX for his help with the cursor postion!
*/
void WriteLinkList(HWND hDlg, BYTE params, LISTELEMENT *listStart, LPCTSTR searchString, int append)
{
	CHARFORMAT2 cf;
	PARAFORMAT2 pf;
	HWND hwndProgress = NULL;
	RECT DesktopRect;
	MYCOLOURSET colourSet;
	TCHAR textLine[LINK_MAX + DIR_SIZE + TIME_SIZE + TYPE_SIZE + 6];
	TCHAR searchText[320];
	TCHAR lastDate[11] = {0};
	TCHAR filter1, filter2, filter3;
	size_t lineLen, listCount=0, realListCount=0, actCount=0, len, appCount = 0;
	int linePos = -1;
	LISTELEMENT *actualElement;
	LISTOPTIONS options;
	CHARRANGE sel; 
	GETTEXTLENGTHEX gtl;

	GetListInfo(params, listStart, searchString, &lineLen, &listCount, &realListCount);
	GetColour(&colourSet);
	GetListOptions(&options);
		
	if ( append == 0 )
		ShowWindow(GetDlgItem(hDlg, IDC_MAIN), SW_HIDE);
	
	if ( (append > 0) && (GetMenuState(GetMenu(hDlg), IDM_SEARCH, MF_BYCOMMAND) & MF_DISABLED))
		return;

	if ( GetDlgItem(hDlg, IDC_MAIN) && GetDlgItem(hDlg, IDC_MESSAGE))
	{
		SendDlgItemMessage( hDlg, IDC_MAIN, EM_SETEVENTMASK, 0, (LPARAM)(ENM_LINK));
		SendDlgItemMessage( hDlg, IDC_MAIN, EM_AUTOURLDETECT, TRUE, 0 );
		SendDlgItemMessage( hDlg, IDC_MAIN, EM_SETBKGNDCOLOR, FALSE, colourSet.background);
		
		memset(&cf, 0, sizeof(cf));
		cf.cbSize = sizeof(cf);
		cf.dwMask = CFM_COLOR;
		cf.crTextColor = colourSet.text;
		SendDlgItemMessage( hDlg, IDC_MESSAGE, EM_SETCHARFORMAT, SCF_SELECTION|SCF_WORD, (LPARAM)&cf);
		SendDlgItemMessage( hDlg, IDC_MESSAGE, EM_AUTOURLDETECT, TRUE, 0 );
		SendDlgItemMessage( hDlg, IDC_MESSAGE, EM_SETBKGNDCOLOR, FALSE, colourSet.background);

		if ( append == 0 )
		{
			ClearLinePos(listStart);
			
			// How to set RTF colour, font, etc.... found at
			// http://www.winehq.com/hypermail/wine-devel/2004/08/0608.html
			memset(&pf, 0, sizeof(pf));
			pf.cbSize = sizeof(pf);
			pf.dwMask = PFM_ALIGNMENT;
			pf.wAlignment = PFA_LEFT;
			SendDlgItemMessage( hDlg, IDC_MAIN,  EM_SETPARAFORMAT, FALSE, (LPARAM) &pf);
		
			if ( searchString != NULL )
			{
				memset(&cf, 0, sizeof(cf));
				cf.cbSize = sizeof(cf);
				cf.dwMask = CFM_ITALIC | CFM_BOLD | CFM_FACE | CFM_COLOR;
				cf.dwEffects = CFE_BOLD;
				cf.crTextColor = colourSet.text;
				_tcscpy_s(cf.szFaceName, _T("Arial"));
				SendDlgItemMessage( hDlg, IDC_MAIN, EM_SETCHARFORMAT, SCF_SELECTION | SCF_WORD, (LPARAM) &cf);
				
				memset(searchText, 0, sizeof(searchText));
				mir_sntprintf(searchText, SIZEOF(searchText), _T("%s '%s': %d\n\n"), TranslateT("Matches for searchtext"), searchString, listCount);
				SendDlgItemMessage(hDlg, IDC_MAIN, EM_REPLACESEL, FALSE, (LPARAM)searchText);
				linePos += 2;
			}
		
			memset(&cf, 0, sizeof(cf));
			cf.cbSize = sizeof(cf);
			cf.dwMask = CFM_FACE | CFM_BOLD;
			_tcscpy_s(cf.szFaceName, _T("Courier"));
			SendDlgItemMessage( hDlg, IDC_MAIN, EM_SETCHARFORMAT, SCF_SELECTION | SCF_WORD, (LPARAM)&cf);
		}

		actualElement = listStart->nextElement;
		
		if ( append > 0 )
		{
			linePos = GetLastLinePos(listStart);

			if ((realListCount - append) == 1)
				_tcscpy_s(lastDate, actualElement->date);

			for(appCount = 1; appCount <= (realListCount - append); appCount++)
			{
				actualElement = actualElement->nextElement;
				if(appCount == (realListCount - append - 1))
					_tcscpy_s(lastDate, actualElement->date);
			}
			gtl.flags = GTL_PRECISE; 
			gtl.codepage = CP_ACP; 
			sel.cpMin = sel.cpMax = SendMessage(GetDlgItem(hDlg, IDC_MAIN), EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 0); 
			SendMessage(GetDlgItem(hDlg, IDC_MAIN), EM_EXSETSEL, 0, (LPARAM) &sel); 
		}

		if ( append == 0 )
		{
			// Create Progressbar
			GetWindowRect(GetDesktopWindow(), &DesktopRect);
			hwndProgress = CreateWindow(_T("Progressbar"), TranslateT("Processing list..."), WS_OVERLAPPED, CW_USEDEFAULT, CW_USEDEFAULT, 350, 45, NULL, NULL, hInst, NULL);
			SetWindowPos(hwndProgress,HWND_TOP,(int)(DesktopRect.right*0.5)-175,(int)(DesktopRect.bottom*0.5)-22,0,0,SWP_NOSIZE);
		
			if ( hwndProgress != NULL )
			{
				ShowWindow(hwndProgress, SW_SHOW);
				SetForegroundWindow(hwndProgress);
			}
		}

		while ( actualElement != NULL )
		{
			filter1 = 0;
			filter2 = 0;
			filter3 = 0;
			
			if ( (params & WLL_IN) && (actualElement->direction == DIRECTION_IN))
				filter1 = 1;
			else if ( (params & WLL_OUT) && (actualElement->direction == DIRECTION_OUT))
				filter1 = 1;
			
			if ( (params & WLL_MAIL) && (actualElement->type == LINK_MAIL))
				filter2 = 1;
			else if ( (params & WLL_URL) && (actualElement->type == LINK_URL))
				filter2 = 1;
			else if ( (params & WLL_FILE) && (actualElement->type == LINK_FILE))
				filter2 = 1;


			if ( searchString != NULL )
			{
				if ( params & SLL_DEEP )
				{	
					// Perform deep scan
					if ( actualElement->hEvent != NULL )
					{
						DBEVENTINFO dbe = { sizeof(dbe) };
						dbe.cbBlob = db_event_getBlobSize(actualElement->hEvent);
						dbe.pBlob = (PBYTE)malloc(dbe.cbBlob+1);
						db_event_get(actualElement->hEvent, &dbe);
						dbe.pBlob[dbe.cbBlob] = 0;
						LPTSTR msg = DbGetEventTextT(&dbe, CP_ACP);
						if ( _tcsstr(msg, searchString))
							filter3 = 1;						
						
						free(dbe.pBlob);
						mir_free(msg);
					}
					else filter3 = 0;
				}
				else if ( _tcsstr(actualElement->link, searchString))
					filter3 = 1;
			}
			else filter3 = 1;

			if ( (filter1 == 1) && (filter2 == 1) && (filter3 == 1))
			{
				LPCTSTR type;

				if ( _tcscmp(actualElement->date, lastDate) != 0 )
				{
					memset(&cf, 0, sizeof(cf));
					cf.cbSize = sizeof(cf);
					cf.dwMask = CFM_COLOR;
					cf.crTextColor = colourSet.text;
					SendDlgItemMessage( hDlg, IDC_MAIN, EM_SETCHARFORMAT, SCF_SELECTION | SCF_WORD, (LPARAM)&cf);
					if ( options.showLine != 0 )
						DrawLine(hDlg, lineLen);
					memset(&cf, 0, sizeof(cf));
					cf.cbSize = sizeof(cf);
					cf.dwMask = CFM_ITALIC | CFM_BOLD | CFM_FACE;
					cf.dwEffects = CFE_BOLD;
					_tcscpy_s(cf.szFaceName, _T("Arial"));
					SendDlgItemMessage( hDlg, IDC_MAIN, EM_SETCHARFORMAT, SCF_SELECTION | SCF_WORD, (LPARAM) &cf);
					if ( options.showDate != 0 )
					{
						SendDlgItemMessage( hDlg, IDC_MAIN, EM_REPLACESEL, FALSE, (LPARAM) actualElement->date);
						SendDlgItemMessage( hDlg, IDC_MAIN, EM_REPLACESEL, FALSE, (LPARAM) _T("\n\n"));
						linePos += 3;
					}
					_tcscpy_s(lastDate, actualElement->date);
					memset(&cf, 0, sizeof(cf));
					cf.cbSize = sizeof cf;
					cf.dwMask = CFM_ITALIC | CFM_BOLD | CFM_UNDERLINE | CFM_FACE;
					_tcscpy_s(cf.szFaceName, _T("Courier"));
					SendDlgItemMessage( hDlg, IDC_MAIN, EM_SETCHARFORMAT, SCF_SELECTION | SCF_WORD, (LPARAM) &cf);
				}
				memset(&cf, 0, sizeof(cf));
				cf.cbSize = sizeof(cf);
				cf.dwMask = CFM_COLOR;
				
				if ( actualElement->direction == DIRECTION_OUT )
				{
					cf.crTextColor = colourSet.incoming;
				}
				else
					cf.crTextColor = colourSet.outgoing;
				
				switch( actualElement->type ) {
				case LINK_MAIL:
					type = _T("[mail]");
					break;
				case LINK_URL:
					type = _T("[URL ]");
					break;
				case LINK_FILE:
					type = _T("[file]");
					break;
				default:
					type = _T("[UNK ]");
				}

				mir_sntprintf(textLine, SIZEOF(textLine), _T("%s%s%s%s%s%s%s\n"),
					options.showDirection ? (actualElement->direction==DIRECTION_IN?_T("[in ]"):_T("[out]")) : _T(""),
					options.showDirection? _T(" ") : _T(""),
					options.showType ? type : _T(""),
					options.showType ? _T("  ") : _T(""),
					options.showTime ? actualElement->time : _T(""),
					options.showTime ? _T(" ") : _T(""),
					actualElement->link);
				SendDlgItemMessage( hDlg, IDC_MAIN, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &cf);
				SendDlgItemMessage(hDlg, IDC_MAIN, EM_REPLACESEL, FALSE,(LPARAM)textLine);
				linePos++;
				actualElement->linePos = linePos;
				actCount++;
				if ( hwndProgress && ( ((int)(((float)actCount/listCount)*100.00)) % 10 == 0 ))
						SendMessage(hwndProgress, WM_COMMAND, 100,((int)(((float)actCount/listCount)*100.00)));

			}
			actualElement = actualElement->nextElement;
		}
		if ( listCount > 0 )
		{
			if ((actCount < listCount) && (append == 0) && (options.showLine != 0))
				DrawLine(hDlg, lineLen);
		}
		else if ( searchString == NULL )
		{	
			memset(&cf, 0, sizeof(cf));
			cf.cbSize = sizeof(cf);
			cf.dwMask = CFM_ITALIC | CFM_BOLD | CFM_FACE;
			cf.dwEffects = CFE_BOLD;
			_tcscpy_s(cf.szFaceName, _T("Arial"));
			SendDlgItemMessage( hDlg, IDC_MAIN, EM_SETCHARFORMAT, SCF_SELECTION | SCF_WORD, (LPARAM) &cf);
			SendDlgItemMessage( hDlg, IDC_MAIN, EM_REPLACESEL, FALSE, (LPARAM)TranslateT("No messages found!\nPlease change current filter options."));
			SendDlgItemMessage( hDlg, IDC_MAIN, EM_REPLACESEL, FALSE, (LPARAM)_T("\n"));
		}
			
	}
	if ( append == 0 )
	{
		SendMessage(hwndProgress, WM_CLOSE, 0, 0);
		ShowWindow(GetDlgItem(hDlg, IDC_MAIN), SW_SHOW);	
	}
	
	PostMessage(GetDlgItem(hDlg, IDC_MAIN), WM_VSCROLL, MAKEWPARAM(SB_BOTTOM, 0), 0);
	len = GetWindowTextLength(GetDlgItem(hDlg, IDC_MAIN));
	PostMessage(GetDlgItem(hDlg, IDC_MAIN), EM_SETSEL, len, len);
}


/*
Output some example text to the options dialog
*/
int WriteOptionExample(HWND hDlg, DWORD InColourSel, DWORD OutColourSel, DWORD BGColourSel, DWORD TxtColourSel, LISTOPTIONS *options)
{
	CHARFORMAT cf;
	PARAFORMAT pf;
	
	memset(&pf, 0, sizeof(pf));
	pf.cbSize = sizeof(pf);
	pf.dwMask = PFM_ALIGNMENT;
	pf.wAlignment = PFA_LEFT;
	SendDlgItemMessage(hDlg, IDC_OPTIONS_RE, EM_SETPARAFORMAT, FALSE, (LPARAM)&pf);
	SendDlgItemMessage(hDlg, IDC_OPTIONS_RE, EM_SETEVENTMASK, 0, (LPARAM)(ENM_LINK));
	SendDlgItemMessage(hDlg, IDC_OPTIONS_RE, EM_AUTOURLDETECT, TRUE, 0);
	SendDlgItemMessage(hDlg, IDC_OPTIONS_RE, EM_SETBKGNDCOLOR, FALSE, BGColourSel);
	SendDlgItemMessage(hDlg, IDC_OPTIONS_RE, WM_SETTEXT , 0, 0);

	memset(&cf, 0, sizeof(cf));
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_FACE | CFM_BOLD | CFM_ITALIC | CFM_COLOR;
	_tcscpy_s(cf.szFaceName, _T("Courier"));
	cf.crTextColor = TxtColourSel;
	SendDlgItemMessage( hDlg, IDC_OPTIONS_RE, EM_SETCHARFORMAT, SCF_SELECTION | SCF_WORD, (LPARAM) &cf);
	if ( options->showLine == 1 )
		SendDlgItemMessage( hDlg, IDC_OPTIONS_RE, EM_REPLACESEL, FALSE, (LPARAM)_T("___________________________________________\n"));
	else
		SendDlgItemMessage( hDlg, IDC_OPTIONS_RE, EM_REPLACESEL, FALSE, (LPARAM)_T("\n"));
	
	memset(&cf, 0, sizeof(cf));
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_BOLD | CFM_FACE | CFM_COLOR;
	cf.dwEffects = CFE_BOLD;
	_tcscpy_s(cf.szFaceName, _T("Arial"));
	cf.crTextColor = TxtColourSel;
	SendDlgItemMessage( hDlg, IDC_OPTIONS_RE, EM_SETCHARFORMAT, SCF_SELECTION | SCF_WORD, (LPARAM) &cf);

	if(options->showDate == 1)
		SendDlgItemMessage( hDlg, IDC_OPTIONS_RE, EM_REPLACESEL, FALSE, (LPARAM)TXT_DATE);
	
	SendDlgItemMessage( hDlg, IDC_OPTIONS_RE, EM_REPLACESEL, FALSE, (LPARAM) _T("\n"));

	memset(&cf, 0, sizeof(cf));
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_ITALIC | CFM_BOLD | CFM_UNDERLINE | CFM_FACE;
	_tcscpy_s(cf.szFaceName, _T("Courier"));
	SendDlgItemMessage( hDlg, IDC_OPTIONS_RE, EM_SETCHARFORMAT, SCF_SELECTION | SCF_WORD, (LPARAM) &cf);

	// incoming
	memset(&cf, 0, sizeof(cf));
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_COLOR | CFM_FACE;
	cf.crTextColor = InColourSel;
	_tcscpy_s(cf.szFaceName, _T("Courier"));
	SendDlgItemMessage(hDlg, IDC_OPTIONS_RE, EM_SETCHARFORMAT, SCF_SELECTION | SCF_WORD, (LPARAM)&cf );
	
	if ( options->showDirection == 1 )
		SendDlgItemMessage(hDlg, IDC_OPTIONS_RE, EM_REPLACESEL, FALSE, (LPARAM)_T("\n[in ] "));
	else
		SendDlgItemMessage(hDlg, IDC_OPTIONS_RE, EM_REPLACESEL, FALSE, (LPARAM)_T("\n"));
		
	if ( options->showType == 1 )
		SendDlgItemMessage(hDlg, IDC_OPTIONS_RE, EM_REPLACESEL, FALSE, (LPARAM)_T("URL   "));
	else
		SendDlgItemMessage(hDlg, IDC_OPTIONS_RE, EM_REPLACESEL, FALSE, (LPARAM)_T(""));
	
	if ( options->showTime == 1 )
		SendDlgItemMessage(hDlg, IDC_OPTIONS_RE, EM_REPLACESEL, FALSE, (LPARAM)_T("08:15 http://miranda-ng.org\n"));
	else
		SendDlgItemMessage(hDlg, IDC_OPTIONS_RE, EM_REPLACESEL, FALSE, (LPARAM)_T("http://miranda-ng.org\n"));

	// outgoing
	memset(&cf, 0, sizeof(cf));
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_COLOR | CFM_FACE;
	cf.crTextColor = OutColourSel;
	_tcscpy_s(cf.szFaceName, _T("Courier"));
	SendDlgItemMessage(hDlg, IDC_OPTIONS_RE, EM_SETCHARFORMAT, SCF_SELECTION | SCF_WORD, (LPARAM)&cf);

	
	if ( options->showDirection == 1 )
		SendDlgItemMessage(hDlg, IDC_OPTIONS_RE, EM_REPLACESEL, FALSE, (LPARAM)_T("[out] "));
	else
		SendDlgItemMessage(hDlg, IDC_OPTIONS_RE, EM_REPLACESEL, FALSE, (LPARAM)_T(""));
		
	if(options->showType == 1)
		SendDlgItemMessage(hDlg, IDC_OPTIONS_RE, EM_REPLACESEL, FALSE, (LPARAM)_T("URL   "));
	else
		SendDlgItemMessage(hDlg, IDC_OPTIONS_RE, EM_REPLACESEL, FALSE, (LPARAM)_T(""));
	
	if(options->showTime == 1)
		SendDlgItemMessage(hDlg, IDC_OPTIONS_RE, EM_REPLACESEL, FALSE, (LPARAM)_T("08:16 http://miranda-ng.org\n"));
	else
		SendDlgItemMessage(hDlg, IDC_OPTIONS_RE, EM_REPLACESEL, FALSE, (LPARAM)_T("http://miranda-ng.org\n"));
	
	return 0;
}


/*
Write Message to window
*/
void WriteMessage(HWND hDlg, LISTELEMENT *listStart, int actLinePos)
{
	LISTELEMENT *actualElement = listStart->nextElement;
	while (actualElement != NULL) {
		if (actualElement->linePos == actLinePos) {
			HANDLE hEvent = actualElement->hEvent;
			if (hEvent != NULL ) {
				DBEVENTINFO dbe = { sizeof(dbe) };
				dbe.cbBlob = db_event_getBlobSize(hEvent);
				dbe.pBlob = (PBYTE)malloc(dbe.cbBlob+1);
				db_event_get(hEvent, &dbe);
				dbe.pBlob[dbe.cbBlob] = 0;
				LPCTSTR msg = DbGetEventTextT(&dbe, CP_ACP);
				SendDlgItemMessage(hDlg, IDC_MESSAGE, WM_SETTEXT , 0, 0);
				SendDlgItemMessage(hDlg, IDC_MESSAGE, EM_REPLACESEL, FALSE, (LPARAM)msg);
				mir_free((void*)msg);
				free(dbe.pBlob);
			}
			break;
		}
		actualElement = actualElement->nextElement;
	}
}

/*
Little helper functions to get the actual state of
user options.
*/
BYTE GetFlags(HMENU listMenu)
{
	BYTE returnflags = 0x00;
	
	if ( GetMenuState(listMenu, IDM_TYPE_WEB, MF_BYCOMMAND) == MF_UNCHECKED )
		returnflags = returnflags | WLL_MAIL;
	
	if ( GetMenuState(listMenu, IDM_TYPE_MAIL, MF_BYCOMMAND) == MF_UNCHECKED )
		returnflags = returnflags | WLL_URL;
	
	if ( GetMenuState(listMenu, IDM_DIR_IN, MF_BYCOMMAND) == MF_UNCHECKED )
		returnflags = returnflags | WLL_OUT;
	
	if ( GetMenuState(listMenu, IDM_DIR_OUT, MF_BYCOMMAND) == MF_UNCHECKED )
		returnflags = returnflags | WLL_IN;
	
	return returnflags;
}

void GetFilterText(HMENU listMenu, LPTSTR filter, size_t max_len)
{

	if ( GetMenuState(listMenu, IDM_TYPE_WEB, MF_BYCOMMAND) == MF_CHECKED )
	{
		if ( GetMenuState(listMenu, IDM_DIR_IN, MF_BYCOMMAND) == MF_CHECKED )
		{
			//incoming URLs
			mir_sntprintf(filter, max_len, _T("%s: %s %s"), TXT_FILTER, TXT_INCOMING, TXT_URLSONLY);
		}
		else if ( GetMenuState(listMenu, IDM_DIR_OUT, MF_BYCOMMAND) == MF_CHECKED )
		{
			//outgoing URLs
			mir_sntprintf(filter, max_len, _T("%s: %s %s"), TXT_FILTER, TXT_OUTGOING, TXT_URLSONLY);
		}
		else
		{
			// both directions (URL)
			mir_sntprintf(filter, max_len, _T("%s: %s %s"), TXT_FILTER, "", TXT_URLSONLY);
		}
	}
	else if ( GetMenuState(listMenu, IDM_TYPE_MAIL, MF_BYCOMMAND) == MF_CHECKED )
	{
		if ( GetMenuState(listMenu, IDM_DIR_IN, MF_BYCOMMAND) == MF_CHECKED )
		{
			//incoming mail
			mir_sntprintf(filter, max_len, _T("%s: %s %s"), TXT_FILTER, TXT_INCOMING, TXT_MAILSONLY);
		}
		else if ( GetMenuState(listMenu, IDM_DIR_OUT, MF_BYCOMMAND) == MF_CHECKED )
		{
			//outgoing mail
			mir_sntprintf(filter, max_len, _T("%s: %s %s"), TXT_FILTER, TXT_OUTGOING, TXT_MAILSONLY);
		}
		else
		{
			// both directions (mail)
			mir_sntprintf(filter, max_len, _T("%s: %s %s"), TXT_FILTER, "", TXT_MAILSONLY);
		}
	}
	else
	{
		if ( GetMenuState(listMenu, IDM_DIR_IN, MF_BYCOMMAND) == MF_CHECKED )
		{
			//incoming (both)
			mir_sntprintf(filter, max_len, _T("%s: %s %s"), TXT_FILTER, TXT_INCOMING, "");
		}
		else if ( GetMenuState(listMenu, IDM_DIR_OUT, MF_BYCOMMAND) == MF_CHECKED )
		{
			//outgoing (both)
			mir_sntprintf(filter, max_len, _T("%s: %s %s"), TXT_FILTER, TXT_OUTGOING, "");
		}
		else
		{
			// no filter
			mir_sntprintf(filter, max_len, _T("%s: %s %s"), TXT_FILTER, TXT_NOFILTER, "");
		}
	}
}


/*
Little helper function to draw a horizontal line
*/
void DrawLine(HWND hDlg, size_t lineLen)
{
	TCHAR line[LINK_MAX + 18];
	size_t i;
	for (i=0; (i<lineLen+18)&&(i<LINK_MAX+18); i++ )
	{
		line[i] = _T('_');
	}
	line[lineLen+18] = _T('\0');
	SendDlgItemMessage( hDlg, IDC_MAIN, EM_REPLACESEL, FALSE, (LPARAM)line);
	SendDlgItemMessage( hDlg, IDC_MAIN, EM_REPLACESEL, FALSE, (LPARAM)_T("\n"));
	return;
}


/*
Little helper function to get informations about the linked list, such as number of links, etc
*/
void GetListInfo(BYTE params, LISTELEMENT *listStart,  LPCTSTR searchString, size_t *maxLen, size_t *elementCount, size_t *realElementCount)
{
	size_t tempLen;		
	LISTELEMENT *actualElement;
	char filter1, filter2, filter3;

	*maxLen = 0;
	*elementCount = 0;
	*realElementCount = 0;
	actualElement = listStart->nextElement;
	
	while ( actualElement != NULL )
	{
		(*realElementCount)++;

		filter1 = 0;
		filter2 = 0;
		filter3 = 0;
			
		if ( (params & WLL_IN) && (actualElement->direction == DIRECTION_IN))
			filter1 = 1;
		else if ( (params & WLL_OUT) && (actualElement->direction == DIRECTION_OUT))
			filter1 = 1;
			
		if ( (params & WLL_MAIL) && (actualElement->type == LINK_MAIL))
			filter2 = 1;
		else if ( (params & WLL_URL) && (actualElement->type == LINK_URL))
			filter2 = 1;
		
		if ( searchString != NULL )
		{
			if ( params & SLL_DEEP )
			{	
				// Perform deep scan
				if ( actualElement->hEvent != NULL )
				{
					DBEVENTINFO dbe = { sizeof(dbe) };
					dbe.cbBlob = db_event_getBlobSize(actualElement->hEvent);
					dbe.pBlob = (PBYTE)malloc(dbe.cbBlob+1);
					db_event_get(actualElement->hEvent, &dbe);
					dbe.pBlob[dbe.cbBlob] = 0;
					if ( _tcsstr((LPTSTR)dbe.pBlob, searchString))
						filter3 = 1;						
					
					free(dbe.pBlob);
				}
				else
					filter3 = 0;
			}
			else
			{
				if(_tcsstr(actualElement->link, searchString))
					filter3 = 1;
			}
		}
		else
			filter3 = 1;

		if ((filter1 == 1) && (filter2 == 1) && (filter3 == 1))	
		{
			(*elementCount)++;

			tempLen = _tcslen(actualElement->link);
			if (*maxLen < tempLen)
				*maxLen = tempLen;
		}
		actualElement = actualElement->nextElement;
	}
	return;
}



void GetListOptions(LISTOPTIONS *options)
{

	options->openNewWindow = db_get_b(NULL, LINKLIST_MODULE, LINKLIST_OPEN_WINDOW, 0xFF);
	if(options->openNewWindow == 0xFF)
	{
		// No DB entry for this Plugin
		db_set_b(NULL, LINKLIST_MODULE, LINKLIST_OPEN_WINDOW, 0x00);
		options->openNewWindow = 0x00;
	}

	
	options->updateWindow = db_get_b(NULL, LINKLIST_MODULE, LINKLIST_UPDATE_WINDOW, 0xFF);
	if(options->updateWindow == 0xFF)
	{
		// No DB entry for this Plugin
		db_set_b(NULL, LINKLIST_MODULE, LINKLIST_UPDATE_WINDOW, 0x00);
		options->updateWindow = 0x00;
	}

	options->mouseEvent = db_get_b(NULL, LINKLIST_MODULE, LINKLIST_MOUSE_EVENT, 0xFF);
	if(options->mouseEvent == 0xFF)
	{
		// No DB entry for this Plugin
		db_set_b(NULL, LINKLIST_MODULE, LINKLIST_MOUSE_EVENT, 0x00);
		options->mouseEvent = 0x00;
	}

	options->saveSpecial = db_get_b(NULL, LINKLIST_MODULE, LINKLIST_SAVESPECIAL, 0xFF);
	if(options->saveSpecial == 0xFF)
	{
		// No DB entry for this Plugin
		db_set_b(NULL, LINKLIST_MODULE, LINKLIST_SAVESPECIAL, 0x00);
		options->saveSpecial = 0x00;
	}


	options->showDate = db_get_b(NULL, LINKLIST_MODULE, LINKLIST_SHOW_DATE, 0xFF);
	if(options->showDate == 0xFF)
	{
		// No DB entry for this Plugin
		db_set_b(NULL, LINKLIST_MODULE, LINKLIST_SHOW_DATE, 0x01);
		options->showDate = 0x01;
	}

	options->showLine = db_get_b(NULL, LINKLIST_MODULE, LINKLIST_SHOW_LINE, 0xFF);
	if(options->showLine == 0xFF)
	{
		// No DB entry for this Plugin
		db_set_b(NULL, LINKLIST_MODULE, LINKLIST_SHOW_LINE, 0x01);
		options->showLine = 0x01;
	}

	options->showTime = db_get_b(NULL, LINKLIST_MODULE, LINKLIST_SHOW_TIME, 0xFF);
	if(options->showTime == 0xFF)
	{
		// No DB entry for this Plugin
		db_set_b(NULL, LINKLIST_MODULE, LINKLIST_SHOW_TIME, 0x01);
		options->showTime = 0x01;
	}

	options->showDirection = db_get_b(NULL, LINKLIST_MODULE, LINKLIST_SHOW_DIRECTION, 0xFF);
	if(options->showDirection == 0xFF)
	{
		// No DB entry for this Plugin
		db_set_b(NULL, LINKLIST_MODULE, LINKLIST_SHOW_DIRECTION, 0x01);
		options->showDirection = 0x01;
	}

	options->showType = db_get_b(NULL, LINKLIST_MODULE, LINKLIST_SHOW_TYPE, 0xFF);
	if(options->showType == 0xFF)
	{
		// No DB entry for this Plugin
		db_set_b(NULL, LINKLIST_MODULE, LINKLIST_SHOW_TYPE, 0x01);
		options->showType = 0x01;
	}

	return;
}

void SetListOptions(LISTOPTIONS *options)
{
	db_set_b(NULL, LINKLIST_MODULE, LINKLIST_OPEN_WINDOW, options->openNewWindow);
	db_set_b(NULL, LINKLIST_MODULE, LINKLIST_UPDATE_WINDOW, options->updateWindow);
	db_set_b(NULL, LINKLIST_MODULE, LINKLIST_MOUSE_EVENT, options->mouseEvent);
	db_set_b(NULL, LINKLIST_MODULE, LINKLIST_SAVESPECIAL, options->saveSpecial);
	db_set_b(NULL, LINKLIST_MODULE, LINKLIST_SHOW_DATE, options->showDate);
	db_set_b(NULL, LINKLIST_MODULE, LINKLIST_SHOW_LINE, options->showLine);
	db_set_b(NULL, LINKLIST_MODULE, LINKLIST_SHOW_TIME, options->showTime);
	db_set_b(NULL, LINKLIST_MODULE, LINKLIST_SHOW_DIRECTION, options->showDirection);
	db_set_b(NULL, LINKLIST_MODULE, LINKLIST_SHOW_TYPE, options->showType);
}

/*
Clear temporary stored Linenumbers in List
*/
void ClearLinePos(LISTELEMENT *listStart)
{
	LISTELEMENT *actualElement;

	if ( listStart == NULL )
		return;

	actualElement = listStart->nextElement;
	while ( actualElement != NULL )
	{
		actualElement->linePos = -1;
		actualElement = actualElement->nextElement;
	}
}

int GetLastLinePos(LISTELEMENT *listStart)
{
	LISTELEMENT *actualElement;
	int maxPos = -1;

	if ( listStart == NULL )
		return -1;

	actualElement = listStart->nextElement;
	while ( actualElement != NULL )
	{
		if ( actualElement->linePos > maxPos )
			maxPos = actualElement->linePos;

		actualElement = actualElement->nextElement;
	}
	return maxPos;
}

/*
Read current coloursettings from the database
*/
void GetColour(MYCOLOURSET *colourSet)
{
	DWORD colour;
	BYTE useDefault;

	useDefault = db_get_b(NULL, LINKLIST_MODULE, LINKLIST_USE_DEF, 0xFF);
	if ( useDefault == 0xFF )
	{
		// No DB entry for this Plugin
		db_set_b(NULL, LINKLIST_MODULE, LINKLIST_USE_DEF, 0x01);
		useDefault = 0x01;
	}

	if ( useDefault == 0x01 )
	{
		// Use Miranda-IM Default colours
		// CHANGED AT MIRANDA 0.4!!!!
		// Use SRMM... if it is not there try SRMsg (older Miranda Versions)
		colour = db_get_dw(NULL, "SRMM", "SRMFont1Col", 0xFF000000);
		if ( colour != 0xFF000000 )
			colourSet->incoming = colour;
		else
		{
			colour = db_get_dw(NULL, "SRMsg", "Font3Col", 0xFF000000);
			if ( colour != 0xFF000000 )
				colourSet->incoming = colour;
			else
			{
				db_set_b(NULL, LINKLIST_MODULE, LINKLIST_USE_DEF, 0x00);
				useDefault = 0x00;
			}
		}
		
		// SRMM
		colour = db_get_dw(NULL, "SRMM", "SRMFont0Col", 0xFF000000);
		if ( colour != 0xFF000000 )
			colourSet->outgoing = colour;
		else
		{
			// SRMsg
			colour = db_get_dw(NULL, "SRMsg", "Font0Col", 0xFF000000);
			if ( colour != 0xFF000000 )
				colourSet->outgoing = colour;
			else
			{
				db_set_b(NULL, LINKLIST_MODULE, LINKLIST_USE_DEF, 0x00);
				useDefault = 0x00;
			}
		}

		// SRMM
		colour = db_get_dw(NULL, "SRMM", "BkgColour", 0xFF000000);
		if ( colour != 0xFF000000 )
			colourSet->background = colour;
		else
		{	
			// SRMsg
			colour = db_get_dw(NULL, "SRMsg", "BkgColour", 0xFF000000);
			if ( colour != 0xFF000000 )
				colourSet->background = colour;
			else
			{
				db_set_b(NULL, LINKLIST_MODULE, LINKLIST_USE_DEF, 0x00);
				useDefault = 0x00;
			}
		}
		colourSet->text = MAKE_TXT_COL(colourSet->background);
	}

	if ( useDefault == 0x00 )
	{
		// Use Plugin user defined or default colours
		colour = db_get_dw(NULL, LINKLIST_MODULE, LINKLIST_IN_COL, 0xFF000000);
		if ( colour != 0xFF000000 )
			colourSet->incoming = colour;
		else
			colourSet->incoming = IN_COL_DEF;
	
		colour = db_get_dw(NULL, LINKLIST_MODULE, LINKLIST_OUT_COL, 0xFF000000);
		if ( colour != 0xFF000000 )
			colourSet->outgoing = colour;
		else
			colourSet->outgoing = OUT_COL_DEF;
	
		colour = db_get_dw(NULL, LINKLIST_MODULE, LINKLIST_BG_COL, 0xFF000000);
		if ( colour != 0xFF000000 )
			colourSet->background = colour;
		else
			colourSet->background = BG_COL_DEF;

		colour = db_get_dw(NULL, LINKLIST_MODULE, LINKLIST_TXT_COL, 0xFF000000);
		if ( colour != 0xFF000000 )
			colourSet->text = colour;
		else
			colourSet->text = TXT_COL_DEF;
	}
}

/*
Read current coloursettings from the database and set default values
if entry does not exist.
*/
void GetDBColour(MYCOLOURSET *colourSet)
{
	DWORD colour;

	// Use Plugin user defined or default colours
	colour = db_get_dw(NULL, LINKLIST_MODULE, LINKLIST_IN_COL, 0xFF000000);
	if(colour != 0xFF000000)
		colourSet->incoming = colour;
	else
	{
		db_set_dw(NULL, LINKLIST_MODULE, LINKLIST_IN_COL, IN_COL_DEF);
		colourSet->incoming = IN_COL_DEF;
	}

	colour = db_get_dw(NULL, LINKLIST_MODULE, LINKLIST_OUT_COL, 0xFF000000);
	if(colour != 0xFF000000)
		colourSet->outgoing = colour;
	else
	{	
		db_set_dw(NULL, LINKLIST_MODULE, LINKLIST_OUT_COL, OUT_COL_DEF);
		colourSet->outgoing = OUT_COL_DEF;
	}

	colour = db_get_dw(NULL, LINKLIST_MODULE, LINKLIST_BG_COL, 0xFF000000);
	if(colour != 0xFF000000)
		colourSet->background = colour;
	else
	{
		db_set_dw(NULL, LINKLIST_MODULE, LINKLIST_BG_COL, BG_COL_DEF);
		colourSet->background = BG_COL_DEF;
	}

	colour = db_get_dw(NULL, LINKLIST_MODULE, LINKLIST_TXT_COL, 0xFF000000);
	if(colour != 0xFF000000)
		colourSet->text = colour;
	else
	{
		db_set_dw(NULL, LINKLIST_MODULE, LINKLIST_TXT_COL, TXT_COL_DEF);
		colourSet->text = TXT_COL_DEF;
	}
}

/*
Read current coloursettings from the database (Miranda settings)
*/
int GetMirandaColour(MYCOLOURSET *colourSet)
{
	DWORD colour;

	// Use Miranda-IM Default colours
	// Try SRMM (Miranda 0.4) .... or SRMsg... for older versions
	colour = db_get_dw(NULL, "SRMM", "SRMFont1Col", 0xFF000000);
	if(colour != 0xFF000000)
		colourSet->incoming = colour;
	else
	{
		colour = db_get_dw(NULL, "SRMsg", "Font3Col", 0xFF000000);
		if(colour != 0xFF000000)
			colourSet->incoming = colour;
		else
			return 1;	
	}
		

	colour = db_get_dw(NULL, "SRMM", "SRMFont0Col", 0xFF000000);
	if(colour != 0xFF000000)
		colourSet->outgoing = colour;
	else
	{
		colour = db_get_dw(NULL, "SRMsg", "Font0Col", 0xFF000000);
		if(colour != 0xFF000000)
			colourSet->outgoing = colour;
		else
			return 1;
	}

	colour = db_get_dw(NULL, "SRMM", "BkgColour", 0xFF000000);
	if(colour != 0xFF000000)
		colourSet->background = colour;
	else
	{	
		colour = db_get_dw(NULL, "SRMsg", "BkgColour", 0xFF000000);
		if(colour != 0xFF000000)
			colourSet->background = colour;
		else
			return 1;
	}

	colourSet->text = MAKE_TXT_COL(colourSet->background);
	return 0;
}


/*
Write user defined colours to the database
*/
void SetDBColour(MYCOLOURSET *colourSet)
{
	db_set_dw(NULL, LINKLIST_MODULE, LINKLIST_IN_COL, colourSet->incoming);
	db_set_dw(NULL, LINKLIST_MODULE, LINKLIST_OUT_COL, colourSet->outgoing);
	db_set_dw(NULL, LINKLIST_MODULE, LINKLIST_BG_COL, colourSet->background);
	db_set_dw(NULL, LINKLIST_MODULE, LINKLIST_TXT_COL, colourSet->text);
}

BYTE GetUpdateSetting(void)
{
	BYTE updateWindow;
	
	updateWindow = db_get_b(NULL, LINKLIST_MODULE, LINKLIST_UPDATE_WINDOW, 0xFF);
	if ( updateWindow == 0xFF )
	{
		// No DB entry for this Plugin
		db_set_b(NULL, LINKLIST_MODULE, LINKLIST_UPDATE_WINDOW, 0x00);
		return 0;
	}
	if ( updateWindow == 0x00 )
		return 0;
	else
		return 1;
}

/*
Special thanks to Tobi H.!
This function is derived from his Wordlookup Plugin
*/
int DBUpdate(WPARAM wParam, LPARAM lParam)
{
	HANDLE hEvent=(HANDLE)lParam;
	HWND hDlg = WindowList_Find(hWindowList, wParam);
	DIALOGPARAM *DlgParam;
	HMENU listMenu = GetMenu(hDlg);
	int linkNum = 0;
	
	DlgParam = (DIALOGPARAM *)GetWindowLongPtr(hDlg, GWLP_USERDATA);

	if(GetUpdateSetting() != 1)
		return 0;

	if(hDlg) {
		DBEVENTINFO dbe = { sizeof(dbe) };
		dbe.cbBlob = db_event_getBlobSize(hEvent);
		dbe.pBlob = (PBYTE)malloc((size_t)dbe.cbBlob+1);
		db_event_get(hEvent, &dbe);
		if (dbe.eventType == EVENTTYPE_URL || dbe.eventType == EVENTTYPE_MESSAGE) {
			// Call function to find URIs
			linkNum = ExtractURI(&dbe, hEvent, DlgParam->listStart);
			if ( linkNum > 0 )
				WriteLinkList(hDlg, GetFlags(listMenu), DlgParam->listStart, NULL, linkNum); 
		}
		free(dbe.pBlob);
	}
	return 0;
}


/*
Little resize helper
*/
int LinklistResizer(HWND hDlg, LPARAM lParam, UTILRESIZECONTROL *urc) 
{
	DIALOGPARAM *DlgParam = (DIALOGPARAM*)lParam;
	UNREFERENCED_PARAMETER(hDlg);
	
	switch(urc->wId) 
	{
		case IDC_MAIN:
			urc->rcItem.bottom -= DlgParam->splitterPosNew - DlgParam->splitterPosOld; 
			return RD_ANCHORX_WIDTH|RD_ANCHORY_HEIGHT;

		case IDC_MESSAGE:
			urc->rcItem.top -= DlgParam->splitterPosNew - DlgParam->splitterPosOld; 
			return RD_ANCHORX_WIDTH|RD_ANCHORY_BOTTOM;
	
		case IDC_STATUS:
			return RD_ANCHORX_WIDTH|RD_ANCHORY_BOTTOM;
		
		case IDC_SPLITTER:
            urc->rcItem.top -= DlgParam->splitterPosNew - DlgParam->splitterPosOld;
            urc->rcItem.bottom -= DlgParam->splitterPosNew - DlgParam->splitterPosOld;
            return RD_ANCHORX_WIDTH|RD_ANCHORY_BOTTOM;
	}

	return RD_ANCHORX_LEFT|RD_ANCHORY_TOP;
}


/*
Next both functions are taken from a example projekt, found at 
http://www.programmersheaven.com/zone15/cat236/2405.htm
The author is unknown, but this peace of code made my work much easier!
*/
BOOL SaveEditAsStream( HWND hDlg )
{
	EDITSTREAM es;
	LONG lOut;
	OPENFILENAME ofn;
	HANDLE hFile;
	TCHAR szFilename[MAX_PATH];

	// Initialize filename field
	_tcscpy_s(szFilename, _T("*.rtf"));
	// Fill in OPENFILENAME struct
	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);      
	ofn.hwndOwner = hDlg;
	TCHAR temp[MAX_PATH]; 
	mir_sntprintf(temp, SIZEOF(temp), _T("%s (*.rtf)%c*.rtf%c%s (*.*)%c*.*%c%c"), TranslateT("RTF file"), 0, 0, TranslateT("All files"), 0, 0, 0);
	ofn.lpstrFilter = temp;
	ofn.lpstrFile = szFilename;
	ofn.nMaxFile = SIZEOF(szFilename);
	ofn.lpstrTitle = TranslateT("Save RTF File");
	ofn.Flags = OFN_OVERWRITEPROMPT;
	// Get a filename or quit
	if ( ! GetSaveFileName( &ofn ))                   
		return FALSE;
	//  Create the specified file
	hFile = CreateFile( szFilename, GENERIC_WRITE, 0, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
	//  Quit if file creation fails
	if ( hFile == INVALID_HANDLE_VALUE )
		return FALSE;
	//  Pass file handle to callback
	//  so the callback can do the file write
	es.dwCookie = (DWORD_PTR)hFile;
	es.dwError = 0;								
	es.pfnCallback = (EDITSTREAMCALLBACK)RTFSaveStreamCallback;
	// Start the callback proc
	lOut = SendDlgItemMessage( hDlg, IDC_MAIN, EM_STREAMOUT, SF_RTF, (LPARAM)&es );
	// Close file handle and exit
	CloseHandle(hFile);
	return TRUE;
}

DWORD CALLBACK RTFSaveStreamCallback(DWORD_PTR dwCookie, LPBYTE lpBuffer, LONG lSize, LONG *plRead)
{
	// Sanity check...exit if nothing passed
	if ( ! lSize )                      
		return 1;
	// Initialize "amount read" variable for WriteFile()
	*plRead = 0;           
	// dwCookie is the file handle
	WriteFile( (HANDLE)dwCookie, lpBuffer, lSize, (LPDWORD)plRead, NULL );
	// Continue, if needed
	return 0;
}


