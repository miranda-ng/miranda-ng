/*
Basic History plugin
Copyright (C) 2011-2012 Krzysztof Kral

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "StdAfx.h"
#include "Searcher.h"
#include "resource.h"

Searcher::Searcher()
	:lastFindSelection(-1),
	findBack(false),
	matchCase(false),
	matchWholeWords(false),
	onlyIn(false),
	onlyOut(false),
	context(NULL)
{
}

void Searcher::ChangeFindDirection(bool isBack)
{
	if (isBack != findBack) {
		findBack = isBack;
		ClearFind();    
		TBBUTTONINFO tbInfo;
		tbInfo.cbSize  = sizeof(TBBUTTONINFO);
		tbInfo.dwMask  = TBIF_TEXT | TBIF_IMAGE;
		if (isBack) {
			tbInfo.pszText = TranslateT("Find Previous");
			tbInfo.iImage = 1;
		}
		else {
			tbInfo.pszText = TranslateT("Find Next");
			tbInfo.iImage = 0;
		}    
		SendMessage(context->toolbarWindow, TB_SETBUTTONINFO, (WPARAM)IDM_FIND, (LPARAM)&tbInfo);
	}

	Find();
}

void Searcher::ClearFind()
{
	if (lastFindSelection != -1) {
		SendMessage(context->editWindow,EM_SETOPTIONS,ECOOP_AND,~ECO_NOHIDESEL);
		lastFindSelection = -1;
	}
}

inline TCHAR mytoupper(TCHAR a, std::locale* loc)
{
	return std::toupper<TCHAR>(a, *loc);
}

bool Searcher::CompareStr(std::wstring str, TCHAR *strFind)
{
	std::locale loc;
	if (!matchCase)
		std::transform(str.begin(), str.end(), str.begin(), std::bind2nd(std::ptr_fun(mytoupper), &loc));
	if (!matchWholeWords)
		return str.find(strFind) < str.length();

	size_t findid = str.find(strFind);
	size_t findLen = _tcslen(strFind);
	while(findid < str.length()) {
		if ((findid == 0 || std::isspace(str[findid - 1], loc) || std::ispunct(str[findid - 1], loc)) &&
			(findid + findLen >= str.length() || std::isspace(str[findid + findLen], loc) || std::ispunct(str[findid + findLen], loc)))
			return true;
		findid = str.find(strFind, findid + 1);
	}

	return false;
}

void Searcher::Find()
{
	FINDTEXTEX ft;
	TCHAR str[128];
	int curSel = 0;
	bool isStart = false;
	bool finished = false;
	ft.chrg.cpMin = 0;
	ft.chrg.cpMax = -1;
	ft.lpstrText = str;
	if (context->currentGroup.size() < 1) {
		SendMessage(context->editWindow,EM_SETOPTIONS,ECOOP_AND,~ECO_NOHIDESEL);
		lastFindSelection = -1;
		return;
	}

	GetWindowText(context->findWindow, str, SIZEOF(str));
	if (!str[0]) {
		TCHAR buf[256];
		mir_sntprintf(buf, SIZEOF(buf), TranslateT("\"%s\" not found"), str);
		MessageBox(context->hWnd, buf, TranslateT("Search"), MB_OK | MB_ICONINFORMATION);
		return;
	}
	if (!matchCase) {
		std::locale loc;
		std::transform(str, str + _tcslen(str), str, std::bind2nd(std::ptr_fun(mytoupper), &loc));
	}
	
	bool findBack1 = findBack ^ !searchForInMes;
	bool findBack2 = findBack ^ !searchForInLG;
	int adder1 = findBack1 ? -1 : 1;
	int adder2 = findBack2 ? -1 : 1;
	WPARAM findStyle = (findBack1 ? 0 : FR_DOWN) | (matchCase ? FR_MATCHCASE : 0) | (matchWholeWords ? FR_WHOLEWORD : 0);
	if (lastFindSelection >= 0 && lastFindSelection < (int)context->currentGroup.size()) {
		if (onlyIn && context->currentGroup[lastFindSelection].isMe || onlyOut && !context->currentGroup[lastFindSelection].isMe)
			curSel = lastFindSelection + adder1;
		else {
			SendDlgItemMessage(context->hWnd,IDC_EDIT,EM_EXGETSEL,0,(LPARAM)&ft.chrg);
			if (findBack1) {
				ft.chrg.cpMin = ft.chrg.cpMin < context->currentGroup[lastFindSelection].endPos ? ft.chrg.cpMin : context->currentGroup[lastFindSelection].endPos; 
				ft.chrg.cpMax = context->currentGroup[lastFindSelection].startPos;
			}
			else {
				ft.chrg.cpMin = ft.chrg.cpMax > context->currentGroup[lastFindSelection].startPos ? ft.chrg.cpMax : context->currentGroup[lastFindSelection].startPos;
				ft.chrg.cpMax = context->currentGroup[lastFindSelection].endPos;
			}
			SendMessage(context->editWindow,EM_FINDTEXTEX, findStyle,(LPARAM)&ft);
			if (ft.chrgText.cpMin < 0 || ft.chrgText.cpMax < 0)
				curSel = lastFindSelection + adder1;
			else {
				if (isFindContactChanged && startFindContact == context->hContact && isFindSelChanged && context->selected == startFindSel && ((!findBack1 && ft.chrg.cpMin >= startFindPos) || (findBack1 && ft.chrg.cpMax <= startFindPos)))
					finished = true;
				else {
					curSel = lastFindSelection;
					SendMessage(context->editWindow,EM_EXSETSEL,0,(LPARAM)&ft.chrgText);
					SendMessage(context->editWindow,EM_SETOPTIONS,ECOOP_OR,ECO_NOHIDESEL);
					lastFindSelection = curSel;
					return;
				}
			}
		}
	}
	else {
		isStart = true;
		SendMessage(context->editWindow,EM_SETOPTIONS,ECOOP_OR,ECO_NOHIDESEL);
		SendMessage(context->editWindow,EM_EXGETSEL,0,(LPARAM)&ft.chrg);
		startFindPos = findBack1 ? ft.chrg.cpMin : (ft.chrg.cpMax >= 0 ? ft.chrg.cpMax : ft.chrg.cpMin);
		startFindSel = context->selected;
		if (startFindPos < 0)
			startFindPos = 0;
		isFindSelChanged = false;
		startFindContact = context->hContact;
		isFindContactChanged = !allUsers;
		if (findBack1) {
			for (curSel = (int)context->currentGroup.size() - 1; curSel >= 0; --curSel)
				if (context->currentGroup[curSel].startPos < startFindPos)
					break;
		}
		else
			for (; curSel < (int)context->currentGroup.size(); ++curSel)
				if (context->currentGroup[curSel].endPos > startFindPos)
					break;
	}

	if (!finished) {
		for (; curSel < (int)context->currentGroup.size() && curSel >= 0; curSel += adder1) {
			if (onlyIn && context->currentGroup[curSel].isMe || onlyOut && !context->currentGroup[curSel].isMe)
				continue;

			if (CompareStr(context->currentGroup[curSel].description, str)) {
				if (findBack1) {
					ft.chrg.cpMin = context->currentGroup[curSel].endPos;
					ft.chrg.cpMax = context->currentGroup[curSel].startPos;
					if (!isFindSelChanged && ft.chrg.cpMin > startFindPos)
						ft.chrg.cpMin = startFindPos;
				}
				else {
					ft.chrg.cpMin = context->currentGroup[curSel].startPos;
					ft.chrg.cpMax = context->currentGroup[curSel].endPos;
					if (!isFindSelChanged && ft.chrg.cpMin < startFindPos)
						ft.chrg.cpMin = startFindPos;
				}
				SendMessage(context->editWindow,EM_FINDTEXTEX, findStyle,(LPARAM)&ft);
				if (!(ft.chrgText.cpMin < 0 || ft.chrgText.cpMax < 0)) {
					if (isFindContactChanged && startFindContact == context->hContact && isFindSelChanged && context->selected == startFindSel && ((!findBack1 && ft.chrg.cpMin >= startFindPos) || (findBack1 && ft.chrg.cpMax <= startFindPos))) {
						finished = true;
						break;
					}
					SendMessage(context->editWindow,EM_EXSETSEL,0,(LPARAM)&ft.chrgText);
					SendMessage(context->editWindow,EM_SETOPTIONS,ECOOP_OR,ECO_NOHIDESEL);
					lastFindSelection = curSel;
					return;
				}
			}
		}
	}
	
	if (isFindContactChanged && startFindContact == context->hContact && isFindSelChanged && context->selected == startFindSel)
		finished = true;

	if (!finished) {
		isFindSelChanged = true;
		if (onlyGroup) {
			if (IsInSel(context->selected, str)) {
				CHARRANGE ch;
				ch.cpMin = ch.cpMax = findBack1 ? MAXLONG : 0;
				SendMessage(context->editWindow,EM_EXSETSEL,0,(LPARAM)&ch);
				lastFindSelection = findBack1 ? (int)context->currentGroup.size() - 1 : 0;
				Find();
				return;
			}
		}
		else {
			for (int sel = context->selected + adder2; ; sel += adder2) {
				if (sel < 0) {
					isFindContactChanged = true;
					if (allUsers) {
						MCONTACT hNext = context->hContact;
						do
						{
							hNext = context->GetNextContact(hNext, adder2);
						}
							while(hNext != startFindContact && !context->SearchInContact(hNext, str, this));
						context->SelectContact(hNext);
					}

					sel = (int)context->eventList.size() - 1;
				}
				else if (sel >= (int)context->eventList.size()) {
					isFindContactChanged = true;
					if (allUsers) {
						MCONTACT hNext = context->hContact;
						do
						{
							hNext = context->GetNextContact(hNext, adder2);
						}
							while(hNext != startFindContact && !context->SearchInContact(hNext, str, this));
						context->SelectContact(hNext);
					}

					sel = 0;
				}
				if (IsInSel(sel, str)) {
					LVITEM item = {0};
					item.mask = LVIF_STATE;
					item.iItem = context->selected;
					item.state = 0;
					item.stateMask = LVIS_SELECTED;
					ListView_SetItem(context->listWindow, &item);
					item.iItem = sel;
					item.state = LVIS_SELECTED;
					ListView_SetItem(context->listWindow, &item);
					ListView_EnsureVisible(context->listWindow, sel, FALSE);		
					CHARRANGE ch;
					ch.cpMin = ch.cpMax = findBack1 ? MAXLONG : 0;
					SendMessage(context->editWindow,EM_EXSETSEL,0,(LPARAM)&ch);
					SendMessage(context->editWindow,EM_SETOPTIONS,ECOOP_OR,ECO_NOHIDESEL);
					lastFindSelection = findBack1 ? (int)context->currentGroup.size() - 1 : 0;
					isFindSelChanged = true;
					Find();
					return;
				}
				if (startFindContact == context->hContact && sel == startFindSel)
					break;
			}
		}
	}
	
	if (startFindContact != context->hContact)
		context->SelectContact(startFindContact);

	if (startFindSel != context->selected) {
		LVITEM item = {0};
		item.mask = LVIF_STATE;
		item.iItem = context->selected;
		item.state = 0;
		item.stateMask = LVIS_SELECTED;
		ListView_SetItem(context->listWindow, &item);
		item.iItem = startFindSel;
		item.state = LVIS_SELECTED;
		ListView_SetItem(context->listWindow, &item);
		ListView_EnsureVisible(context->listWindow, startFindSel, FALSE);			
		context->SelectEventGroup(startFindSel);
		SendMessage(context->editWindow,EM_SETOPTIONS,ECOOP_OR,ECO_NOHIDESEL);
	}
	ft.chrgText.cpMin = startFindPos;
	ft.chrgText.cpMax = startFindPos;
	SendMessage(context->editWindow,EM_EXSETSEL,0,(LPARAM)&ft.chrgText);
	SendMessage(context->editWindow,EM_SETOPTIONS,ECOOP_AND,~ECO_NOHIDESEL);
	lastFindSelection = -1;
	if (isStart) {
		TCHAR buf[256];
		GetWindowText(context->findWindow, str, SIZEOF(str));
		mir_sntprintf(buf, SIZEOF(buf), TranslateT("\"%s\" not found"), str);
		MessageBox(context->hWnd, buf, TranslateT("Search"), MB_OK | MB_ICONINFORMATION);
	}
	else MessageBox(context->hWnd, TranslateTS(onlyGroup ? LPGENT("You have reached the end of the group.") : LPGENT("You have reached the end of the history.")), TranslateT("Search"), MB_OK | MB_ICONINFORMATION);
}

bool Searcher::IsInSel(int sel, TCHAR *strFind)
{
	if (sel < 0 || sel >= (int)context->eventList.size())
		return false;

	TCHAR str[MAXSELECTSTR + 8]; // for safety reason
	EventList::EventData data;
	for (std::deque<EventList::EventIndex>::iterator it = context->eventList[sel].begin(); it != context->eventList[sel].end(); ++it) {
		EventList::EventIndex hDbEvent = *it;
		if (context->GetEventData(hDbEvent, data)) {
			bool isMe = data.isMe;
			if (onlyIn && isMe || onlyOut && !isMe)
				continue;

			context->GetEventMessage(hDbEvent, str);
			if (CompareStr(str, strFind))
				return true;
		}
	}
		
	return false;
}

bool Searcher::Compare(const bool isMe, const std::wstring& message, TCHAR *strFind)
{
	if (onlyIn && isMe || onlyOut && !isMe)
		return false;

	return CompareStr(message, strFind);
}
