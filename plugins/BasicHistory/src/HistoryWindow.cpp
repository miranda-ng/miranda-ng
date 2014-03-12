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
#include "HistoryWindow.h"
#include "resource.h"
#include "Options.h"
#include "HotkeyHelper.h"
#include "ImageDataObject.h"
#include "ExportManager.h"

#define MODULE				"BasicHistory"
extern HINSTANCE hInst;
extern HCURSOR  hCurSplitNS, hCurSplitWE;
extern int iconsNum;
extern bool g_SmileyAddAvail;
extern char* metaContactProto;
extern IconItem iconList[];
#define DM_HREBUILD  (WM_USER+11)
#define DM_SPLITTERMOVED     (WM_USER+15)

#define MIN_PANELHEIGHT 40

void ResetCList(HWND hWnd);

HistoryWindow::HistoryWindow(MCONTACT _hContact) :
	isDestroyed(true),
	splitterY(0),
	splitterOrgY(0),
	splitterX(0),
	splitterOrgX(0),
	plusIco(NULL),
	minusIco(NULL),
	findNextIco(NULL),
	findPrevIco(NULL),
	configIco(NULL),
	deleteIco(NULL),
	isContactList(false),
	isLoading(false),
	isGroupImages(false),
	allIconNumber(0), 
	eventIcons(NULL),
	bkBrush(NULL),
	bkFindBrush(NULL),
	hSystem(NULL),
	splitterXhWnd(NULL),
	splitterYhWnd(NULL),
	isStartSelect(true)
{
	searcher.SetContect(this);
	hContact = _hContact;
	selected = -1;
	searcher.SetMatchCase(Options::instance->searchMatchCase);
	searcher.SetMatchWholeWords(Options::instance->searchMatchWhole);
	searcher.SetOnlyIn(Options::instance->searchOnlyIn);
	searcher.SetOnlyOut(Options::instance->searchOnlyOut);
	searcher.SetOnlyGroup(Options::instance->searchOnlyGroup);
	searcher.SetAllUsers(Options::instance->searchAllContacts);
	searcher.SetSearchForInLG(Options::instance->searchForInList);
	searcher.SetSearchForInMes(Options::instance->searchForInMess);
}


HistoryWindow::~HistoryWindow()
{
	if (eventIcons != NULL)
	{
		for (int i = 0; i < iconsNum; ++i)
			if (eventIcons[i] != NULL)
				Skin_ReleaseIcon(eventIcons[i]);

		delete[] eventIcons;
	}

	if (plusIco != NULL)
		Skin_ReleaseIcon(plusIco);

	if (minusIco != NULL)
		Skin_ReleaseIcon(minusIco);

	if (findNextIco != NULL)
		Skin_ReleaseIcon(findNextIco);

	if (findPrevIco != NULL)
		Skin_ReleaseIcon(findPrevIco);

	if (himlSmall != NULL)
		ImageList_Destroy(himlSmall);

	if (himlNone != NULL)
		ImageList_Destroy(himlNone);

	if (bkBrush != NULL)
		DeleteObject(bkBrush);

	if (bkFindBrush != NULL)
		DeleteObject(bkFindBrush);
}

std::map<MCONTACT, HistoryWindow*> HistoryWindow::windows;
std::vector<HistoryWindow*> HistoryWindow::freeWindows;

void HistoryWindow::Deinit()
{
	bool destroyed = true;
	std::vector<MCONTACT> keys;
	for (std::map<MCONTACT, HistoryWindow*>::iterator it = windows.begin(); it != windows.end(); ++it)
	{
		if (!it->second->isDestroyed)
		{
			keys.push_back(it->first);
		}
	}
	for (std::vector<MCONTACT>::iterator it = keys.begin(); it != keys.end(); ++it)
	{
		std::map<MCONTACT, HistoryWindow*>::iterator it1 = windows.find(*it);
		if (it1 != windows.end())
		{
			DestroyWindow(it1->second->hWnd);
			it1 = windows.find(*it);
			destroyed &= it1 == windows.end();
		}
	}
	
	std::vector<HistoryWindow*> keys1;
	for (std::vector<HistoryWindow*>::iterator it = freeWindows.begin(); it != freeWindows.end(); ++it)
	{
		if (!(*it)->isDestroyed)
		{
			keys1.push_back(*it);
		}
	}
	for (std::vector<HistoryWindow*>::iterator it = keys1.begin(); it != keys1.end(); ++it)
	{
		DestroyWindow((*it)->hWnd);
	}
	for (std::vector<HistoryWindow*>::iterator it = freeWindows.begin(); it != freeWindows.end(); ++it)
	{
		if (!(*it)->isDestroyed)
		{
			destroyed = false;
			break;
		}
	}

	if (destroyed)
	{
		for (std::map<MCONTACT, HistoryWindow*>::iterator it = windows.begin(); it != windows.end(); ++it)
		{
			delete it->second;
		}

		windows.clear();

		for (std::vector<HistoryWindow*>::iterator it = freeWindows.begin(); it != freeWindows.end(); ++it)
		{
			delete *it;
		}

		freeWindows.clear();
	}
}

void HistoryWindow::Open(MCONTACT hContact)
{
	if (hContact == NULL)
	{
		HistoryWindow *hw = new HistoryWindow(hContact);
		freeWindows.push_back(hw);
		hw->Show();
	}
	else
	{
		std::map<MCONTACT, HistoryWindow*>::iterator it = windows.find(hContact);
		if (it != windows.end())
		{
			it->second->Focus();
		}
		else
		{
			windows[hContact] = new HistoryWindow(hContact);
			windows[hContact]->Show();
		}
	}
}

void HistoryWindow::Close(MCONTACT hContact)
{
	std::map<MCONTACT, HistoryWindow*>::iterator it = windows.find(hContact);
	if (it != windows.end())
	{
		if (it->second->isDestroyed)
		{
			delete it->second;
			windows.erase(it);
		}
		else
		{
			DestroyWindow(it->second->hWnd);
		}
	}
}

void  HistoryWindow::Close(HistoryWindow* historyWindow)
{
	if (!historyWindow->isDestroyed)
	{
		DestroyWindow(historyWindow->hWnd);
		return;
	}
	std::map<MCONTACT, HistoryWindow*>::iterator it = windows.find(historyWindow->hContact);
	if (it != windows.end() && it->second == historyWindow)
	{
		delete it->second;
		windows.erase(it);
	}
	else
	{
		for (std::vector<HistoryWindow*>::iterator it = freeWindows.begin(); it != freeWindows.end(); ++it)
		{
			if (*it == historyWindow)
			{
				freeWindows.erase(it);
				delete historyWindow;
				return;
			}
		}
	}
}

void HistoryWindow::RebuildEvents(MCONTACT hContact)
{
	if (hContact != NULL)
	{
		std::map<MCONTACT, HistoryWindow*>::iterator it = windows.find(hContact);
		if (it != windows.end() && !it->second->isDestroyed)
		{
			SendMessage(it->second->hWnd,DM_HREBUILD,0,0);
		}
	}
	
	for (std::vector<HistoryWindow*>::iterator it = freeWindows.begin(); it != freeWindows.end(); ++it)
	{
		if ((*it)->hContact == hContact && !(*it)->isDestroyed)
		{
			SendMessage((*it)->hWnd,DM_HREBUILD,0,0);
		}
	}
}

void HistoryWindow::ChangeToFreeWindow(HistoryWindow* historyWindow)
{
	std::map<MCONTACT, HistoryWindow*>::iterator it = windows.find(historyWindow->hContact);
	if (it != windows.end() && it->second == historyWindow)
	{
		windows.erase(it);
		freeWindows.push_back(historyWindow);
	}
}

void HistoryWindow::Show()
{
	CreateDialogParam(hInst,MAKEINTRESOURCE(IDD_HISTORY),NULL,HistoryWindow::DlgProcHistory,(LPARAM)this);
}

void HistoryWindow::Focus()
{
	if (IsIconic(hWnd))
	{
		ShowWindow(hWnd, SW_RESTORE);
	}
	else
	{
		SetForegroundWindow(hWnd);
		SetFocus(hWnd);
	}
	SendMessage(hWnd,DM_HREBUILD,0,0);
}

int HistoryWindow::FontsChanged(WPARAM wParam, LPARAM lParam)
{
	for (std::map<MCONTACT, HistoryWindow*>::iterator it = windows.begin(); it != windows.end(); ++it)
	{
		if (!it->second->isDestroyed)
		{
			it->second->FontsChanged();
		}
	}

	for (std::vector<HistoryWindow*>::iterator it = freeWindows.begin(); it != freeWindows.end(); ++it)
	{
		if (!(*it)->isDestroyed)
		{
			(*it)->FontsChanged();
		}
	}

	return 0;
}

void HistoryWindow::FontsChanged()
{
	if (bkBrush != NULL)
	{
		DeleteObject(bkBrush);
	}
	if (bkFindBrush != NULL)
	{
		DeleteObject(bkFindBrush);
	}

	bkBrush = CreateSolidBrush(Options::instance->GetColor(Options::WindowBackground));
	bkFindBrush = CreateSolidBrush(Options::instance->GetColor(Options::FindBackground));
	
	ResetCList(hWnd);
	COLORREF bkColor = Options::instance->GetColor(Options::GroupListBackground);
	ListView_SetBkColor(listWindow, bkColor);
	ListView_SetTextBkColor(listWindow, bkColor);
	LOGFONT font;
	ListView_SetTextColor(listWindow, Options::instance->GetFont(Options::GroupList, &font));
	InvalidateRect(listWindow, NULL, TRUE);
	InvalidateRect(hWnd, NULL, TRUE);
	SelectEventGroup(selected);
}

void OptionsGroupChanged()
{
	HistoryWindow::OptionsGroupChanged();
}

void HistoryWindow::OptionsGroupChanged()
{
	for (std::map<MCONTACT, HistoryWindow*>::iterator it = windows.begin(); it != windows.end(); ++it)
	{
		if (!it->second->isDestroyed)
		{
			it->second->GroupImagesChanged();
			SendMessage(it->second->hWnd,DM_HREBUILD,0,0);
		}
	}

	for (std::vector<HistoryWindow*>::iterator it = freeWindows.begin(); it != freeWindows.end(); ++it)
	{
		if (!(*it)->isDestroyed)
		{
			(*it)->GroupImagesChanged();
			SendMessage((*it)->hWnd,DM_HREBUILD,0,0);
		}
	}
}

void OptionsMainChanged()
{
	HistoryWindow::OptionsMainChanged();
}

void HistoryWindow::OptionsMainChanged()
{
	for (std::map<MCONTACT, HistoryWindow*>::iterator it = windows.begin(); it != windows.end(); ++it)
	{
		if (!it->second->isDestroyed)
		{
			it->second->ReloadMainOptions();
		}
	}

	for (std::vector<HistoryWindow*>::iterator it = freeWindows.begin(); it != freeWindows.end(); ++it)
	{
		if (!(*it)->isDestroyed)
		{
			(*it)->ReloadMainOptions();
		}
	}
}

void HistoryWindow::ReloadMainOptions()
{
	SendDlgItemMessage(hWnd, IDC_LIST_CONTACTS, CLM_SETUSEGROUPS, Options::instance->showContactGroups, 0);
	SendMessage(hWnd,DM_HREBUILD,0,0);
}

void OptionsMessageChanged()
{
	HistoryWindow::FontsChanged(0, 0);
}

void OptionsSearchingChanged()
{
	HistoryWindow::OptionsSearchingChanged();
}

void HistoryWindow::OptionsSearchingChanged()
{
	for (std::map<MCONTACT, HistoryWindow*>::iterator it = windows.begin(); it != windows.end(); ++it)
	{
		if (!it->second->isDestroyed)
		{
			it->second->searcher.SetSearchForInLG(Options::instance->searchForInList);
			it->second->searcher.SetSearchForInMes(Options::instance->searchForInMess);
		}
	}

	for (std::vector<HistoryWindow*>::iterator it = freeWindows.begin(); it != freeWindows.end(); ++it)
	{
		if (!(*it)->isDestroyed)
		{
			(*it)->searcher.SetSearchForInLG(Options::instance->searchForInList);
			(*it)->searcher.SetSearchForInMes(Options::instance->searchForInMess);
		}
	}
}

INT_PTR HistoryWindow::DeleteAllUserHistory(WPARAM hContact, LPARAM)
{
	HWND hWnd = NULL;
	int start = 0;
	int end = 0;
	int count = EventList::GetContactMessageNumber(hContact);
	if (!count)
		return FALSE;
	
	for (std::map<MCONTACT, HistoryWindow*>::iterator it = windows.begin(); it != windows.end(); ++it)
	{
		if (!it->second->isDestroyed)
		{
			if (it->second->hContact == hContact)
			{
				if (hWnd == NULL)
				{
					hWnd = it->second->hWnd;
				}
				else if (GetForegroundWindow() == it->second->hWnd)
				{
					hWnd = it->second->hWnd;
				}
			}
		}
	}

	for (std::vector<HistoryWindow*>::iterator it = freeWindows.begin(); it != freeWindows.end(); ++it)
	{
		if (!(*it)->isDestroyed)
		{
			if ((*it)->hContact == hContact)
			{
				if (hWnd == NULL)
				{
					hWnd = (*it)->hWnd;
				}
				else if (GetForegroundWindow() == (*it)->hWnd)
				{
					hWnd = (*it)->hWnd;
				}
			}
		}
	}

	TCHAR *message = TranslateT("This operation will PERMANENTLY REMOVE all history for this contact.\nAre you sure you want to do this?");
	if (MessageBox(hWnd, message, TranslateT("Are You sure?"), MB_OKCANCEL | MB_ICONERROR) != IDOK)
		return FALSE;

	CallService(MS_DB_SETSAFETYMODE, FALSE, 0);
	HANDLE hDbEvent = db_event_last(hContact);
	while (hDbEvent != NULL) {
		HANDLE hPrevEvent = db_event_prev(hContact, hDbEvent);
		hDbEvent = ( db_event_delete(hContact, hDbEvent) == 0) ? hPrevEvent : NULL;
	}
	CallService(MS_DB_SETSAFETYMODE, TRUE, 0);

	if (EventList::IsImportedHistory(hContact)) {
		TCHAR *message = TranslateT("Do you want to delete all imported messages for this contact?\nNote that next scheduler task import this messages again.");
		if (MessageBox(hWnd, message, TranslateT("Are You sure?"), MB_YESNO | MB_ICONERROR) == IDYES)
			EventList::DeleteImporter(hContact);
	}
		
	RebuildEvents(hContact);
	return TRUE;
}

bool HistoryWindow::IsInList(HWND hWnd)
{
	for (std::map<MCONTACT, HistoryWindow*>::iterator it = windows.begin(); it != windows.end(); ++it)
		if (!it->second->isDestroyed)
			if (it->second->hWnd == hWnd)
				return true;

	for (std::vector<HistoryWindow*>::iterator it = freeWindows.begin(); it != freeWindows.end(); ++it)
		if (!(*it)->isDestroyed)
			if ((*it)->hWnd == hWnd)
				return true;
	
	return false;
}

void ClickLink(HWND hwnd, ENLINK *penLink)
{
	TCHAR buf[1024];
	if (penLink->msg != WM_LBUTTONUP)
		return;

	if (penLink->chrg.cpMin >= 0 && penLink->chrg.cpMax > penLink->chrg.cpMin) {
		// selection
		int len = penLink->chrg.cpMax - penLink->chrg.cpMin;
		if (len < 1023) {
			TEXTRANGE tr;
			CHARRANGE sel;

			SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM) & sel);
			if (sel.cpMin != sel.cpMax)
				return;
			tr.chrg = penLink->chrg;
			tr.lpstrText = buf;
			SendMessage(hwnd, EM_GETTEXTRANGE, 0, (LPARAM) & tr);
			CallService(MS_UTILS_OPENURL, (penLink->nmhdr.code == IDM_OPENNEW ? OUF_NEWWINDOW : 0) | OUF_TCHAR, (LPARAM) tr.lpstrText);
		}
	}
}

void ConvertSize(HWND hwndSrc, HWND hwndDest, RECT& rc)
{
	POINT pt;
	pt.x = rc.left;
	pt.y = rc.top;
	ClientToScreen(hwndSrc, &pt);
	ScreenToClient(hwndDest, &pt);
	rc.left = pt.x;
	rc.top = pt.y;

	pt.x = rc.right;
	pt.y = rc.bottom;
	ClientToScreen(hwndSrc, &pt);
	ScreenToClient(hwndDest, &pt);
	rc.right = pt.x;
	rc.bottom = pt.y;
}

void OpenOptions(char* group, char* page, char* tab = NULL)
{
	OPENOPTIONSDIALOG op;
	op.cbSize = sizeof(OPENOPTIONSDIALOG);
	op.pszGroup = group;
	op.pszPage = page;
	op.pszTab = tab;
	Options_Open(&op);
}

#define DlgReturn(ret){\
	SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (ret));\
	return (ret);\
}

class ShowMessageData
{
public:
	ShowMessageData(MCONTACT _hContact)
		:hContact(_hContact)
	{
	}

	ShowMessageData(MCONTACT _hContact, const std::wstring &_str)
		:hContact(_hContact),
		str(_str)
	{
	}

	MCONTACT hContact;
	std::wstring str;
};

void __stdcall ShowMessageWindow(void* arg)
{
	ShowMessageData* dt = (ShowMessageData*)arg;
	if (dt->str.empty())
		CallService(MS_MSG_SENDMESSAGE, (WPARAM)dt->hContact, 0);
	else
		CallService(MS_MSG_SENDMESSAGET, (WPARAM)dt->hContact, (LPARAM)dt->str.c_str());
	delete dt;
}

INT_PTR CALLBACK HistoryWindow::DlgProcHistory(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HistoryWindow* historyWindow = (HistoryWindow*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	LPNMHDR pNmhdr;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		historyWindow = (HistoryWindow*)lParam;
		historyWindow->hWnd = hwndDlg;
		historyWindow->isWnd = true;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);
		historyWindow->Initialise();
		DlgReturn(TRUE);

	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 500;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 380;

	case WM_SIZE:
		{
			UTILRESIZEDIALOG urd = { sizeof(urd) };
			urd.hwndDlg = hwndDlg;
			urd.hInstance = hInst;
			urd.lpTemplate = MAKEINTRESOURCEA(IDD_HISTORY);
			urd.pfnResizer = HistoryWindow::HistoryDlgResizer;
			CallService(MS_UTILS_RESIZEDIALOG, 0, (LPARAM)&urd);
			ListView_SetColumnWidth(GetDlgItem(hwndDlg,IDC_LIST), 0, LVSCW_AUTOSIZE_USEHEADER);
		}
		DlgReturn(TRUE);

	case WM_COMMAND:
		switch ( LOWORD( wParam )) {
		case IDOK:
		case IDCANCEL:
			DestroyWindow(hwndDlg);
			DlgReturn(TRUE);

		case IDM_FIND:
			historyWindow->searcher.Find();
			DlgReturn(TRUE);

		case IDM_CONFIG:
			{
				OPENOPTIONSDIALOG opd = {0};
				opd.cbSize = sizeof(opd);
				opd.pszPage = LPGEN("History");
				Options_Open(&opd);
			}
			DlgReturn(TRUE);

		case IDM_DELETE:
			historyWindow->Delete(0);
			DlgReturn(TRUE);

		case IDC_FIND_TEXT:
			if (HIWORD(wParam) == EN_CHANGE)
				historyWindow->searcher.ClearFind();
			DlgReturn(TRUE);

		case IDC_SHOWHIDE:
			if (HIWORD( wParam ) == BN_CLICKED) {
				if (Button_GetCheck(GetDlgItem(hwndDlg,IDC_SHOWHIDE)) & BST_CHECKED) {
					SendDlgItemMessage( hwndDlg, IDC_SHOWHIDE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)historyWindow->minusIco);
					SendDlgItemMessage( hwndDlg, IDC_SHOWHIDE, BUTTONADDTOOLTIP, (WPARAM)LPGENT("Hide Contacts"), BATF_TCHAR);
					historyWindow->isContactList = true;
					ShowWindow(GetDlgItem(hwndDlg,IDC_LIST_CONTACTS), SW_SHOW);
					ShowWindow(historyWindow->splitterYhWnd, SW_SHOW);
				}
				else {
					SendDlgItemMessage( hwndDlg, IDC_SHOWHIDE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)historyWindow->plusIco);
					SendDlgItemMessage( hwndDlg, IDC_SHOWHIDE, BUTTONADDTOOLTIP, (WPARAM)LPGENT("Show Contacts"), BATF_TCHAR);
					historyWindow->isContactList = false;
					ShowWindow(GetDlgItem(hwndDlg,IDC_LIST_CONTACTS), SW_HIDE);
					ShowWindow(historyWindow->splitterYhWnd, SW_HIDE);
				}

				SendMessage(hwndDlg, WM_SIZE, 0, 0);
			}

			DlgReturn(TRUE);
		}
		break;

	case WM_NOTIFY:
		pNmhdr = (LPNMHDR)lParam;
		switch(pNmhdr->idFrom) {
		case IDC_LIST_CONTACTS:
			if (pNmhdr->code == CLN_LISTREBUILT) {
				if (historyWindow != NULL)
					historyWindow->ReloadContacts();

				DlgReturn(TRUE);
			}
			else if (pNmhdr->code == CLN_MYSELCHANGED) {
				if (historyWindow->ContactChanged()) {
					MSGFILTER* msgFilter = (MSGFILTER *) lParam;
					if (msgFilter->msg == WM_LBUTTONDOWN)
						SendMessage(pNmhdr->hwndFrom, WM_LBUTTONUP, msgFilter->wParam, msgFilter->lParam);
				}

				DlgReturn(TRUE);
			}
			else if (pNmhdr->code == CLN_OPTIONSCHANGED) {
				ResetCList(hwndDlg);
				return FALSE;
			}

			//fall through
		case IDC_SHOWHIDE:
		case IDC_FIND_TEXT:
		case IDC_EDIT:
			if (pNmhdr->code == EN_LINK) {
				ClickLink(GetDlgItem(hwndDlg, IDC_EDIT), (ENLINK *) lParam);
				return FALSE;
			}
			if (pNmhdr->code == EN_SELCHANGE)
				historyWindow->searcher.ClearFind();
			else if (pNmhdr->code == EN_MSGFILTER) {
				MSGFILTER* msgFilter = (MSGFILTER *) lParam;
				if (msgFilter->msg == WM_KEYDOWN || msgFilter->msg == WM_SYSKEYDOWN) 
				{
					if (historyWindow->DoHotkey(msgFilter->msg, msgFilter->lParam,  msgFilter->wParam, pNmhdr->idFrom))
						DlgReturn(TRUE);
				}
				else if (msgFilter->msg == WM_RBUTTONDOWN || msgFilter->msg == WM_RBUTTONDBLCLK || msgFilter->msg == WM_NCRBUTTONUP || msgFilter->msg == WM_NCRBUTTONDBLCLK || msgFilter->msg == WM_NCRBUTTONDOWN) 
					DlgReturn(TRUE);

				if (msgFilter->msg == WM_RBUTTONUP) {
					POINT clicked;
					LPNMITEMACTIVATE nmlv = (LPNMITEMACTIVATE)lParam;
					HWND window = historyWindow->editWindow;
					POINTL p;
					POINT scrool;
					LVHITTESTINFO info = {0};
					p.x = clicked.x = info.pt.x = GET_X_LPARAM(msgFilter->lParam);
					p.y = clicked.y = info.pt.y = GET_Y_LPARAM(msgFilter->lParam);
					ClientToScreen(window, &clicked);
					SetFocus(window);
					int selChar = SendMessage(window, EM_CHARFROMPOS, 0, (LPARAM)&p);
					CHARRANGE chrg;
					SendMessage(window,EM_EXGETSEL,0,(LPARAM)&chrg);
					SendMessage(window,EM_GETSCROLLPOS,0,(LPARAM)&scrool);
					if (selChar < chrg.cpMin || selChar > chrg.cpMax)
						chrg.cpMin = chrg.cpMax = selChar;

					if (chrg.cpMin == chrg.cpMax) {
						CHARRANGE chrgNew;
						chrgNew.cpMin = chrg.cpMin;
						chrgNew.cpMax = chrg.cpMax + 1;
						SendMessage(window,EM_EXSETSEL,0,(LPARAM)&chrgNew);
					}
					CHARFORMAT2 chf;
					memset(&chf, 0, sizeof(CHARFORMAT2));
					chf.cbSize = sizeof(CHARFORMAT2);
					chf.dwMask = CFM_LINK;
					SendMessage(window, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&chf);
					if (chrg.cpMin == chrg.cpMax)
						SendMessage(window,EM_EXSETSEL,0,(LPARAM)&chrg);

					HMENU hPopupMenu = CreatePopupMenu();
					if (hPopupMenu != NULL) {
						if (chf.dwEffects & CFE_LINK) {
							AppendMenu(hPopupMenu, MF_STRING, IDM_OPENNEW, TranslateT("Open in &new window"));
							AppendMenu(hPopupMenu, MF_STRING, IDM_OPENEXISTING, TranslateT("&Open in existing window"));
							AppendMenu(hPopupMenu, MF_STRING, IDM_COPYLINK, TranslateT("&Copy link"));
						}
						else {
							AppendMenu(hPopupMenu, MF_STRING, IDM_COPY, TranslateT("Copy"));
							AppendMenu(hPopupMenu, MF_STRING, IDM_DELETE, TranslateT("Delete"));
							AppendMenu(hPopupMenu, MFT_SEPARATOR, 0, NULL);
							AppendMenu(hPopupMenu, MF_STRING, IDM_MESSAGE, TranslateT("Send Message"));
							AppendMenu(hPopupMenu, MF_STRING, IDM_QUOTE, TranslateT("Reply &Quoted"));
							AppendMenu(hPopupMenu, MF_STRING, IDM_DELETEGROUP, TranslateT("Delete Group"));
							AppendMenu(hPopupMenu, MF_STRING, IDM_DELETEUSER, TranslateT("Delete All User History"));
						}

						int selected = TrackPopupMenu(hPopupMenu, TPM_RETURNCMD, clicked.x, clicked.y, 0, hwndDlg, 0);
						switch (selected) {
						case IDM_COPY:
							if (chrg.cpMax == chrg.cpMin && historyWindow->currentGroup.size() > 0) {
								size_t start = 0;
								while(start < historyWindow->currentGroup.size() && chrg.cpMin >= historyWindow->currentGroup[start].endPos) ++start;
								if (start < historyWindow->currentGroup.size()) {
									CHARRANGE chrgNew;
									chrgNew.cpMin = 0;
									if (start > 0)
										chrgNew.cpMin = historyWindow->currentGroup[start - 1].endPos;
									chrgNew.cpMax = historyWindow->currentGroup[start].endPos;
									SendMessage(window,EM_EXSETSEL,0,(LPARAM)&chrgNew);
									SendMessage(window,WM_COPY,0,0);
									SendMessage(window,EM_EXSETSEL,0,(LPARAM)&chrg);
								}
							}
							else SendMessage(window,WM_COPY,0,0);
							break;

						case IDM_MESSAGE:
							CallFunctionAsync(ShowMessageWindow, new ShowMessageData(historyWindow->hContact));
							break;

						case IDM_QUOTE:
							if (historyWindow->currentGroup.size() > 0) {
								std::wstring quote;
								if (chrg.cpMax == chrg.cpMin) {
									size_t start = 0;
									while(start < historyWindow->currentGroup.size() && chrg.cpMin >= historyWindow->currentGroup[start].endPos) ++start;
									if (start < historyWindow->currentGroup.size())
										historyWindow->FormatQuote(quote, historyWindow->currentGroup[start], historyWindow->currentGroup[start].description);
								}
								else {
									size_t start = 0;
									while(start < historyWindow->currentGroup.size() && chrg.cpMin >= historyWindow->currentGroup[start].endPos) ++start;
									size_t end = 0;
									while(end < historyWindow->currentGroup.size() && chrg.cpMax > historyWindow->currentGroup[end].endPos) ++end;
									if (end >= historyWindow->currentGroup.size())
										end = historyWindow->currentGroup.size() - 1;
									if (start == end && start < historyWindow->currentGroup.size()) {
										int iStart = historyWindow->currentGroup[start].startPos;
										if (chrg.cpMin > iStart)
											iStart = chrg.cpMin;
										int iEnd = historyWindow->currentGroup[start].endPos;
										if (chrg.cpMax < iEnd)
											iEnd = chrg.cpMax;
										if (iEnd > iStart) {
											TEXTRANGE tr;
											tr.chrg.cpMin = iStart;
											tr.chrg.cpMax = iEnd;
											tr.lpstrText = new TCHAR[iEnd - iStart + 1];
											SendMessage(historyWindow->editWindow, EM_GETTEXTRANGE, 0, (LPARAM) & tr);
											historyWindow->FormatQuote(quote, historyWindow->currentGroup[start], tr.lpstrText);
											delete [] tr.lpstrText;
										}
									}
									else {
										while(start <= end) {
											historyWindow->FormatQuote(quote, historyWindow->currentGroup[start], historyWindow->currentGroup[start].description);
											++start;
										}
									}
								}

								if (!quote.empty())
									CallFunctionAsync(ShowMessageWindow, new ShowMessageData(historyWindow->hContact, quote));
							}
							break;
						case IDM_DELETE:
							historyWindow->Delete(0);
							break;
						case IDM_DELETEGROUP:
							historyWindow->Delete(1);
							break;
						case IDM_DELETEUSER:
							historyWindow->Delete(2);
							break;
						case IDM_OPENNEW:
						case IDM_OPENEXISTING:
						case IDM_COPYLINK:
							{
								int start = chrg.cpMin, end = chrg.cpMin;
								CHARRANGE chrgNew;
								chrgNew.cpMin = start-1;
								chrgNew.cpMax = start;
								do
								{
									memset(&chf, 0, sizeof(CHARFORMAT2));
									chf.cbSize = sizeof(CHARFORMAT2);
									chf.dwMask = CFM_LINK;
									int sel = SendMessage(window,EM_EXSETSEL,0,(LPARAM)&chrgNew);
									if (sel != chrgNew.cpMax)
										break;
									SendMessage(window, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&chf);
									--chrgNew.cpMin;
									--chrgNew.cpMax;
									--start;
								} while(start >= 0 && chf.dwEffects & CFE_LINK);

								++start;
								chrgNew.cpMin = end;
								chrgNew.cpMax = end + 1;
								do
								{
									memset(&chf, 0, sizeof(CHARFORMAT2));
									chf.cbSize = sizeof(CHARFORMAT2);
									chf.dwMask = CFM_LINK;
									int sel = SendMessage(window,EM_EXSETSEL,0,(LPARAM)&chrgNew);
									if (sel != chrgNew.cpMax)
										break;
									SendMessage(window, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&chf);
									++chrgNew.cpMin;
									++chrgNew.cpMax;
									++end;
								} while(chf.dwEffects & CFE_LINK);

								--end;
								if (selected == IDM_COPYLINK) {
									chrgNew.cpMin = start;
									chrgNew.cpMax = end;
									SendMessage(window,EM_EXSETSEL,0,(LPARAM)&chrgNew);
									SendMessage(window,WM_COPY,0,0);
									SendMessage(window,EM_EXSETSEL,0,(LPARAM)&chrg);
								}
								else {
									ENLINK link;
									link.chrg.cpMin = start;
									link.chrg.cpMax = end;
									link.msg = WM_LBUTTONUP;
									link.nmhdr.code = selected;
									SendMessage(window,EM_EXSETSEL,0,(LPARAM)&chrg);
									ClickLink(window, &link);
								}
							}
							break;
						}

						DestroyMenu(hPopupMenu);
					}
					SendMessage(window,EM_SETSCROLLPOS,0,(LPARAM)&scrool);
					DlgReturn(TRUE);
				}
			}
			break;

		case IDC_LIST:
			if (pNmhdr->code == LVN_ITEMCHANGED) {
				NMLISTVIEW  *nmlv = (NMLISTVIEW*)lParam;
				if ((nmlv->uChanged & LVIF_STATE) && (nmlv->uNewState & LVIS_SELECTED) && historyWindow->selected != nmlv->iItem && nmlv->iItem >= 0) {
					historyWindow->SelectEventGroup(nmlv->iItem);
					DlgReturn(TRUE);
				}
			}
			else if (pNmhdr->code == LVN_KEYDOWN) {
				LPNMLVKEYDOWN  nmlv = (LPNMLVKEYDOWN)lParam;
				if (historyWindow->DoHotkey(WM_KEYDOWN, 0, nmlv->wVKey, IDC_LIST))
					DlgReturn(TRUE);
			}
			else if (pNmhdr->code ==  NM_RCLICK) {
				POINT clicked;
				LPNMITEMACTIVATE nmlv = (LPNMITEMACTIVATE)lParam;
				HWND window = historyWindow->listWindow;
				LVHITTESTINFO info = {0};
				clicked.x = info.pt.x = nmlv->ptAction.x;
				clicked.y = info.pt.y = nmlv->ptAction.y;
				ClientToScreen(window, &clicked);
				int newSel = SendMessage(window, LVM_SUBITEMHITTEST, 0, (LPARAM)&info);
				int curSel = historyWindow->selected;

				if (newSel >= 0) { 
					HMENU hPopupMenu = CreatePopupMenu();
					if (hPopupMenu != NULL) {
						AppendMenu(hPopupMenu, MF_STRING, IDM_COPY, TranslateT("Copy"));
						AppendMenu(hPopupMenu, MF_STRING, IDM_DELETEGROUP, TranslateT("Delete Group"));
						AppendMenu(hPopupMenu, MFT_SEPARATOR, 0, NULL);
						AppendMenu(hPopupMenu, MF_STRING, IDM_MESSAGE, TranslateT("Send Message"));
						AppendMenu(hPopupMenu, MF_STRING, IDM_DELETEUSER, TranslateT("Delete All User History"));

						int selected = TrackPopupMenu(hPopupMenu, TPM_RETURNCMD, clicked.x, clicked.y, 0, hwndDlg, 0);
						switch (selected) {
						case IDM_COPY:
							{
								CHARRANGE chrg;
								SendMessage(historyWindow->editWindow,EM_EXGETSEL,0,(LPARAM)&chrg);
								CHARRANGE chrgNew;
								chrgNew.cpMin = 0;
								chrgNew.cpMax = -1;
								SendMessage(historyWindow->editWindow,EM_EXSETSEL,0,(LPARAM)&chrgNew);
								SendMessage(historyWindow->editWindow,WM_COPY,0,0);
								SendMessage(historyWindow->editWindow,EM_EXSETSEL,0,(LPARAM)&chrg);
							}
							break;
						case IDM_MESSAGE:
							CallService(MS_MSG_SENDMESSAGE, (WPARAM)historyWindow->hContact, 0);
							break;
						case IDM_DELETEGROUP:
							historyWindow->Delete(1);
							break;
						case IDM_DELETEUSER:
							historyWindow->Delete(2);
							break;
						}

						DestroyMenu(hPopupMenu);
					}
				}

				DlgReturn(TRUE);
			}
			break;

		case IDC_TOOLBAR:        
			if (pNmhdr->code == TBN_DROPDOWN) {   
				LPNMTOOLBAR lpnmTB= (LPNMTOOLBAR)lParam;
				if (lpnmTB->iItem == IDM_FIND)
					historyWindow->FindToolbarClicked(lpnmTB);
				else if (lpnmTB->iItem == IDM_CONFIG)
					historyWindow->ConfigToolbarClicked(lpnmTB);
				else if (lpnmTB->iItem == IDM_DELETE)
					historyWindow->DeleteToolbarClicked(lpnmTB);

				DlgReturn(TBDDRET_DEFAULT);
			}
			else if (pNmhdr->code == NM_KEYDOWN) {
				LPNMKEY nmlv = (LPNMKEY)lParam;
				if (historyWindow->DoHotkey(WM_KEYDOWN, 0, nmlv->nVKey, IDC_TOOLBAR))
					DlgReturn(TRUE);
			}
		}
		break;

	case WM_CTLCOLORDLG:
		DlgReturn((LONG_PTR)historyWindow->bkBrush);

	case WM_CTLCOLORSTATIC:
		{
			HWND curhWnd = (HWND)lParam;
			if (historyWindow->splitterXhWnd == curhWnd || historyWindow->splitterYhWnd == curhWnd)
				DlgReturn((LONG_PTR)historyWindow->bkBrush);

			break;
		}
	case WM_CTLCOLOREDIT:
		{
			HWND curhWnd = (HWND)lParam;
			if (historyWindow->findWindow == curhWnd) {
				HDC edithdc = (HDC)wParam;
				LOGFONT font;
				SetTextColor(edithdc, Options::instance->GetFont(Options::Find, &font));
				SetBkColor(edithdc, Options::instance->GetColor(Options::FindBackground));
				DlgReturn((LONG_PTR)historyWindow->bkFindBrush);
			}
		}	
		break;

	case DM_SPLITTERMOVED: 
		historyWindow->SplitterMoved((HWND)lParam, wParam, true);
		break;

	case DM_HREBUILD:
		if (!historyWindow->isLoading) {
			historyWindow->isLoading = true;
			historyWindow->ReloadContacts();
			mir_forkthread(HistoryWindow::FillHistoryThread, historyWindow);
		}
		DlgReturn(TRUE);

	case WM_DESTROY:
		historyWindow->Destroy();
		DlgReturn(TRUE);
	}
	return FALSE;
}

void HistoryWindow::Initialise()
{
	splitterXhWnd = GetDlgItem(hWnd, IDC_SPLITTER);
	splitterYhWnd = GetDlgItem(hWnd, IDC_SPLITTERV);
	mir_subclassWindow(splitterXhWnd, SplitterSubclassProc);
	mir_subclassWindow(splitterYhWnd, SplitterSubclassProc);

	editWindow = GetDlgItem(hWnd, IDC_EDIT);
	findWindow = GetDlgItem(hWnd, IDC_FIND_TEXT);
	toolbarWindow = GetDlgItem(hWnd, IDC_TOOLBAR);
	listWindow = GetDlgItem(hWnd, IDC_LIST);

	RECT rc;
	POINT pt;
	GetWindowRect(splitterXhWnd, &rc);
	pt.y = (rc.top + rc.bottom) / 2;
	pt.x = 0;
	ScreenToClient(hWnd, &pt);
	splitterOrgY = pt.y;
	splitterY = pt.y;
	GetWindowRect(splitterYhWnd, &rc);
	pt.y = 0;
	pt.x = (rc.left + rc.right) / 2;
	ScreenToClient(hWnd, &pt);
	splitterOrgX = pt.x;
	splitterX = pt.x;
	GetWindowRect(GetDlgItem(hWnd, IDC_LIST_CONTACTS), &rc);
	pt.y = rc.top;
	pt.x = rc.left;
	ScreenToClient(hWnd, &pt);
	listOryginalPos = pt.x;

	plusIco = LoadIconEx(IDI_SHOW, 1);
	minusIco = LoadIconEx(IDI_HIDE, 1);
	SendDlgItemMessage( hWnd, IDC_SHOWHIDE, BUTTONSETASPUSHBTN, TRUE, 0 );
	SendDlgItemMessage( hWnd, IDC_SHOWHIDE, BUTTONSETASFLATBTN, TRUE, 0 );
	if (hContact == NULL || Options::instance->showContacts)
	{
		SendDlgItemMessage( hWnd, IDC_SHOWHIDE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)minusIco);
		SendDlgItemMessage( hWnd, IDC_SHOWHIDE, BUTTONADDTOOLTIP, (WPARAM)LPGENT("Hide Contacts"), BATF_TCHAR);
		Button_SetCheck(GetDlgItem(hWnd,IDC_SHOWHIDE), BST_CHECKED);
		isContactList = true;
	}
	else
	{
		SendDlgItemMessage( hWnd, IDC_SHOWHIDE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)plusIco);
		SendDlgItemMessage( hWnd, IDC_SHOWHIDE, BUTTONADDTOOLTIP, (WPARAM)LPGENT("Show Contacts"), BATF_TCHAR);
		Button_SetCheck(GetDlgItem(hWnd,IDC_SHOWHIDE), BST_UNCHECKED);
		ShowWindow(GetDlgItem(hWnd,IDC_LIST_CONTACTS), SW_HIDE);
		ShowWindow(splitterYhWnd, SW_HIDE);
		isContactList = false;
	}
	RegisterHotkeyControl(GetDlgItem(hWnd, IDC_SHOWHIDE));
	RegisterHotkeyControl(GetDlgItem(hWnd, IDC_LIST_CONTACTS));

	ResetCList(hWnd);
			
	RestorePos();
	SendMessage(hWnd, WM_SETICON, ICON_BIG,   ( LPARAM )LoadSkinnedIconBig( SKINICON_OTHER_HISTORY ));
	SendMessage(hWnd, WM_SETICON, ICON_SMALL, ( LPARAM )LoadSkinnedIcon( SKINICON_OTHER_HISTORY ));
	SendMessage(editWindow,EM_AUTOURLDETECT,TRUE,0);
	SendMessage(editWindow,EM_SETEVENTMASK,0,ENM_LINK | ENM_SELCHANGE | ENM_KEYEVENTS | ENM_MOUSEEVENTS);
	SendMessage(editWindow,EM_SETEDITSTYLE,SES_EXTENDBACKCOLOR,SES_EXTENDBACKCOLOR);
			
	himlSmall = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 2, 2);
	himlNone = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 2, 2);
	ImageList_SetIconSize(himlNone, 0, 16);
	if (himlSmall)
	{
		allIconNumber = iconsNum + 3;
		eventIcons = new HICON[allIconNumber];
		for (int i = 0; i < iconsNum; ++i) {
			eventIcons[i] = Skin_GetIconByHandle( iconList[i].hIcolib );
			ImageList_AddIcon(himlSmall, eventIcons[i]);
		}

		int id = iconsNum;
		eventIcons[id] = LoadSkinnedIcon(SKINICON_EVENT_FILE);
		ImageList_AddIcon(himlSmall, eventIcons[id]);

		eventIcons[++id] = LoadSkinnedIcon(SKINICON_EVENT_URL);
		ImageList_AddIcon(himlSmall, eventIcons[id]);

		eventIcons[++id] = LoadSkinnedIcon(SKINICON_OTHER_WINDOWS);
		ImageList_AddIcon(himlSmall, eventIcons[id]);

		if ((isGroupImages = Options::instance->groupShowEvents) != false)
			ListView_SetImageList(listWindow, himlSmall, LVSIL_SMALL);
	}
	
	bkBrush = CreateSolidBrush(Options::instance->GetColor(Options::WindowBackground));
	bkFindBrush  = CreateSolidBrush(Options::instance->GetColor(Options::FindBackground));

	LVCOLUMN col = {0};
	col.mask = LVCF_WIDTH | LVCF_TEXT;
	col.cx = 470;
	col.pszText = _T("");
	ListView_InsertColumn(listWindow, 0, &col);
	ListView_SetColumnWidth(listWindow, 0, LVSCW_AUTOSIZE_USEHEADER);
	ListView_SetExtendedListViewStyleEx(listWindow, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
	COLORREF bkColor = Options::instance->GetColor(Options::GroupListBackground);
	ListView_SetBkColor(listWindow, bkColor);
	ListView_SetTextBkColor(listWindow, bkColor);
	LOGFONT font;
	ListView_SetTextColor(listWindow, Options::instance->GetFont(Options::GroupList, &font));

	Edit_LimitText(findWindow, 100);
	RegisterHotkeyControl(findWindow);
			
	HIMAGELIST himlButtons = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 3, 3);
	if (himlButtons)
	{
		findNextIco = LoadIconEx(IDI_FINDNEXT);
		ImageList_AddIcon(himlButtons, findNextIco);
		findPrevIco = LoadIconEx(IDI_FINDPREV);
		ImageList_AddIcon(himlButtons, findPrevIco);
		configIco = LoadSkinnedIcon(SKINICON_OTHER_OPTIONS);
		ImageList_AddIcon(himlButtons, configIco);
		deleteIco = LoadSkinnedIcon(SKINICON_OTHER_DELETE);
		ImageList_AddIcon(himlButtons, deleteIco);
				
		// Set the image list.
		SendMessage(toolbarWindow, TB_SETIMAGELIST, 0, (LPARAM)himlButtons);

		// Load the button images.
		SendMessage(toolbarWindow, TB_LOADIMAGES, (WPARAM)IDB_STD_SMALL_COLOR, (LPARAM)HINST_COMMCTRL);
	}

	TBBUTTON tbButtons[] = 
	{
		{ 0, IDM_FIND,  TBSTATE_ENABLED, BTNS_DROPDOWN, {0}, 0, (INT_PTR)TranslateT("Find Next") },
		{ 3, IDM_DELETE, TBSTATE_ENABLED,  BTNS_DROPDOWN, {0}, 0, (INT_PTR)TranslateT("Delete")},
		{ 2, IDM_CONFIG, TBSTATE_ENABLED,  BTNS_DROPDOWN, {0}, 0, (INT_PTR)TranslateT("Options")},
	};    
	SendMessage(toolbarWindow, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
	SendMessage(toolbarWindow, TB_ADDBUTTONS,       (WPARAM)SIZEOF(tbButtons),       (LPARAM)&tbButtons);
	SendMessage(toolbarWindow, TB_SETBUTTONSIZE, 0, MAKELPARAM(16, 16));
	SendMessage(toolbarWindow, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_DRAWDDARROWS);
	SendMessage(toolbarWindow, TB_SETMAXTEXTROWS, 0, 0);

	SetDefFilter(Options::instance->defFilter);
			
	InvalidateRect(listWindow, NULL, TRUE);
	InvalidateRect(hWnd, NULL, TRUE);
	SendMessage(hWnd, DM_SETDEFID, IDM_FIND, 0);
	SendMessage(hWnd, WM_SIZE, 0, 0);
	SendMessage(hWnd,DM_HREBUILD,0,0);
	isDestroyed = false;
}

void HistoryWindow::Destroy()
{
	HICON hIcon = (HICON)SendMessage(hWnd, WM_SETICON, ICON_BIG, 0);
	Skin_ReleaseIcon(hIcon);
	
	hIcon = (HICON)SendMessage(hWnd, WM_SETICON, ICON_SMALL, 0);
	Skin_ReleaseIcon(hIcon);

	isDestroyed = true;
	HistoryWindow::Close(this);
}

void HistoryWindow::SplitterMoved(HWND splitter, LONG pos, bool screenPos)
{
	POINT pt;
	RECT rc1;
	RECT rc2;
	POINT pt1;
	POINT pt2;
			
	if (splitter == splitterXhWnd)
	{
		GetWindowRect(listWindow, &rc1);
		GetWindowRect(editWindow, &rc2);
		pt.x = 0;
		pt.y = pos;
		pt1.x = rc1.left;
		pt1.y = rc1.top;
		pt2.x = rc2.right;
		pt2.y = rc2.bottom;
		if (screenPos)
			ScreenToClient(hWnd, &pt);
		ScreenToClient(hWnd, &pt1);
		ScreenToClient(hWnd, &pt2);
		if ((pt.y >= pt1.y + MIN_PANELHEIGHT) && (pt.y < pt2.y - MIN_PANELHEIGHT))
		{
			splitterY = pt.y;
			if (!screenPos)
				SendMessage(hWnd, WM_SIZE, 0, 0);
			//if (M->isAero())
			//	InvalidateRect(GetParent(hwndDlg), NULL, FALSE);
		}
	}
	else
	{
		GetWindowRect(GetDlgItem(hWnd, IDC_LIST_CONTACTS), &rc1);
		GetWindowRect(listWindow, &rc2);
		pt.x = pos;
		pt.y = 0;
		pt1.x = rc1.left;
		pt1.y = rc1.top;
		pt2.x = rc2.right;
		pt2.y = rc2.bottom;
		if (screenPos)
			ScreenToClient(hWnd, &pt);
		ScreenToClient(hWnd, &pt1);
		ScreenToClient(hWnd, &pt2);
		if ((pt.x >= pt1.x + MIN_PANELHEIGHT) && (pt.x < pt2.x - MIN_PANELHEIGHT))
		{
			splitterX = pt.x;
			if (!screenPos)
				SendMessage(hWnd, WM_SIZE, 0, 0);
			//if (M->isAero())
			//	InvalidateRect(GetParent(hwndDlg), NULL, FALSE);
		}
	}
}

int HistoryWindow::HistoryDlgResizer(HWND hwnd, LPARAM, UTILRESIZECONTROL *urc)
{
	HistoryWindow* historyWindow =(HistoryWindow*)GetWindowLongPtr(hwnd,GWLP_USERDATA);
	switch(urc->wId) {
	case IDC_LIST:
		{
			urc->rcItem.bottom += historyWindow->splitterY - historyWindow->splitterOrgY;
			urc->rcItem.left += historyWindow->splitterX - historyWindow->splitterOrgX;
			if (!historyWindow->isContactList)
				urc->rcItem.left = historyWindow->listOryginalPos;
			return RD_ANCHORX_WIDTH|RD_ANCHORY_TOP;
		}
	case IDC_LIST_CONTACTS:
		{
			urc->rcItem.right += historyWindow->splitterX - historyWindow->splitterOrgX;
			return RD_ANCHORX_LEFT|RD_ANCHORY_HEIGHT;
		}
	case IDC_SPLITTER:
		{
			urc->rcItem.top += historyWindow->splitterY - historyWindow->splitterOrgY;
			urc->rcItem.bottom += historyWindow->splitterY - historyWindow->splitterOrgY;
			urc->rcItem.left += historyWindow->splitterX - historyWindow->splitterOrgX;
			if (!historyWindow->isContactList)
				urc->rcItem.left = 0;
			return RD_ANCHORX_WIDTH|RD_ANCHORY_TOP;
		}
	case IDC_SPLITTERV:
		{
			urc->rcItem.left += historyWindow->splitterX - historyWindow->splitterOrgX;
			urc->rcItem.right += historyWindow->splitterX - historyWindow->splitterOrgX;
			return RD_ANCHORX_LEFT|RD_ANCHORY_HEIGHT;
		}
	case IDC_EDIT:
		{
			urc->rcItem.top += historyWindow->splitterY - historyWindow->splitterOrgY;
			urc->rcItem.left += historyWindow->splitterX - historyWindow->splitterOrgX;
			if (!historyWindow->isContactList)
				urc->rcItem.left = historyWindow->listOryginalPos;
			return RD_ANCHORX_WIDTH|RD_ANCHORY_HEIGHT;
		}
	case IDC_FIND_TEXT:
		return RD_ANCHORX_WIDTH|RD_ANCHORY_TOP;
	case IDC_SHOWHIDE:
		return RD_ANCHORX_LEFT|RD_ANCHORY_TOP;
	case IDC_TOOLBAR:
		return RD_ANCHORX_RIGHT|RD_ANCHORY_TOP;
	}
	return RD_ANCHORX_LEFT|RD_ANCHORY_TOP;
}

void HistoryWindow::FillHistoryThread(void* param)
{
	HistoryWindow *hInfo = ( HistoryWindow* )param;
	HWND hwndList = hInfo->listWindow;
	ListView_DeleteAllItems(hwndList);
	hInfo->SelectEventGroup(-1);
	hInfo->EnableWindows(FALSE);
	bool isNewOnTop = Options::instance->groupNewOnTop;

	hInfo->RefreshEventList();

	LVITEM item = {0};
	item.mask = LVIF_STATE;
	item.iItem = 0;
	item.state = LVIS_SELECTED;
	item.stateMask = LVIS_SELECTED;
	if (!isNewOnTop)
	{
		item.iItem = ListView_GetItemCount(hwndList) - 1;
		if (item.iItem < 0)
			item.iItem = 0;
	}

	ListView_SetItem(hwndList, &item);
	ListView_SetColumnWidth(hwndList, 0, LVSCW_AUTOSIZE_USEHEADER);
	ListView_EnsureVisible(hwndList, item.iItem, FALSE);
	hInfo->EnableWindows(TRUE);
	SetFocus(hwndList);
}

void HistoryWindow::AddGroup(bool isMe, const std::wstring &time, const std::wstring &user, const std::wstring &eventText, int ico)
{
	TCHAR msg[256];
	msg[0] = 0;
	if (Options::instance->groupShowTime)
	{
		_tcscpy_s(msg, time.c_str());
	}
	if (Options::instance->groupShowName)
	{
		if (msg[0] != 0)
			_tcscat_s(msg, _T(" "));
		_tcscat_s(msg, user.c_str());
	}

	if (Options::instance->groupShowMessage)
	{
		if (msg[0] != 0)
			_tcscat_s(msg, _T(" "));
		_tcscat_s(msg, eventText.c_str());
	}

	LVITEM item = {0};
	item.mask = LVIF_TEXT | LVIF_IMAGE;
	item.iItem = MAXINT;
	item.pszText = msg;
	item.iImage = ico;
	ListView_InsertItem(listWindow, &item);
}

void HistoryWindow::ReplaceIcons(HWND hwndDlg, int selStart, BOOL isSent)
{
	if (g_SmileyAddAvail)
	{
		CHARRANGE sel;
		SMADD_RICHEDIT3 smadd = {0};

		sel.cpMin = selStart;
		sel.cpMax = -1;

		smadd.cbSize = sizeof(smadd);
		smadd.hwndRichEditControl = hwndDlg;
		smadd.Protocolname = GetContactProto(hContact);
		smadd.hContact = hContact;
		smadd.flags = isSent ? SAFLRE_OUTGOING : 0;
		if (selStart > 0)
			smadd.rangeToReplace = &sel;
		CallService(MS_SMILEYADD_REPLACESMILEYS, 0, (LPARAM)&smadd);
	}
}

void SetFontFromOptions(ITextFont *TextFont, int caps, Options::Fonts fontId)
{
	COLORREF fontColor;
	LOGFONT font;
	fontColor = Options::instance->GetFont(fontId, &font);
	BSTR bstrFont = SysAllocString(font.lfFaceName);
	TextFont->SetName(bstrFont);
	SysFreeString(bstrFont);
	TextFont->SetForeColor(fontColor);
	TextFont->SetWeight(font.lfWeight);
	font.lfHeight = (font.lfHeight * 72) / caps;
	TextFont->SetSize(font.lfHeight < 0 ? -font.lfHeight : font.lfHeight);
	TextFont->SetItalic(font.lfItalic ? tomTrue : tomFalse);
	TextFont->SetItalic(font.lfUnderline ? tomTrue : tomFalse);
	TextFont->SetItalic(font.lfStrikeOut ? tomTrue : tomFalse);
}

void HistoryWindow::SelectEventGroup(int sel)
{
	SendMessage(editWindow, WM_SETTEXT, 0, (LPARAM)_T(""));
	currentGroup.clear();
	selected = sel;
	if (sel < 0 || sel >= (int)eventList.size())
		return;

	TCHAR _str[MAXSELECTSTR + 8]; // for safety reason
	TCHAR *str = _str + sizeof(int)/sizeof(TCHAR);
	BSTR pStr = str;
	unsigned int *strLen = (unsigned int*)_str;
	str[0] = 0;
	tm lastTime;
	bool isFirst = true;
	bool lastMe = false;
	long startAt, endAt;
	long cnt;
	std::wstring strStl;
	IRichEditOle* RichEditOle;
	EventData data;
	if (SendMessage(editWindow, EM_GETOLEINTERFACE, 0, (LPARAM)&RichEditOle) == 0)
		return;
	ITextDocument* TextDocument;
	if (RichEditOle->QueryInterface(IID_ITextDocument, (void**)&TextDocument) != S_OK)
	{
		RichEditOle->Release();
		return;
	}
	ITextSelection* TextSelection;
	ITextFont *TextFont;
	SendMessage(editWindow, EM_SETREADONLY, FALSE, 0);
	TextDocument->Freeze(&cnt);
	TextDocument->GetSelection(&TextSelection);
	HDC hDC =  GetDC(NULL);
	int caps = GetDeviceCaps(hDC, LOGPIXELSY);
	std::deque<EventIndex> revDeq;
	std::deque<EventIndex>& deq = eventList[sel];
	if (Options::instance->messagesNewOnTop)
	{
		revDeq.insert(revDeq.begin(), deq.rbegin(), deq.rend());
		deq = revDeq;
	}
	COLORREF backColor = GetSysColor(COLOR_WINDOW);
	for (std::deque<EventIndex>::iterator it = deq.begin(); it != deq.end(); ++it)
	{
		EventIndex hDbEvent = *it;
		if (GetEventData(hDbEvent, data))
		{
			bool isUser = Options::instance->messagesShowName && (isFirst || (!lastMe && data.isMe) || (lastMe && !data.isMe));
			lastMe = data.isMe;
			backColor = Options::instance->GetColor(lastMe ? Options::OutBackground : Options::InBackground);
			if (Options::instance->messagesShowEvents)
			{
				str[0] = _T('>');
				str[1] = 0;
				*strLen = 1 * sizeof(TCHAR);
				TextSelection->SetStart(MAXLONG);
				TextSelection->GetFont(&TextFont);
				TextFont->SetBackColor(backColor);
				TextSelection->SetText(pStr);
				TextFont->Release();
				int imId;
				HICON ico;
				if (GetEventIcon(lastMe, data.eventType, imId))
				{
					ico = eventIcons[imId];
				}
				else
				{
					ico = GetEventCoreIcon(hDbEvent);
					if (ico == NULL)
					{
						ico = eventIcons[imId];
					}
				}

				ImageDataObject::InsertIcon(RichEditOle, ico, backColor, 16, 16);
			}

			TCHAR* formatDate = Options::instance->messagesShowSec ? (isUser ? _T("d s ") : _T("d s\n")) : (isUser ? _T("d t ") : _T("d t\n"));
			if (!Options::instance->messagesShowDate)
			{
				if (isFirst)
				{
					isFirst = false;
					formatDate = Options::instance->messagesShowSec ? (isUser ? _T("s ") : _T("s\n")) : (isUser ? _T("t ") : _T("t\n"));
					time_t tt = data.timestamp;
					localtime_s(&lastTime, &tt);
				}
				else
				{
					time_t tt = data.timestamp;
					tm t;
					localtime_s(&t, &tt);
					if (lastTime.tm_yday == t.tm_yday && lastTime.tm_year == t.tm_year)
						formatDate = Options::instance->messagesShowSec ? (isUser ? _T("s ") : _T("s\n")) : (isUser ? _T("t ") : _T("t\n"));
				}
			}

			tmi.printTimeStamp(NULL, data.timestamp, formatDate, str , MAXSELECTSTR, 0);
			*strLen = (unsigned int)_tcslen(str) * sizeof(TCHAR);
			TextSelection->SetStart(MAXLONG);
			TextSelection->GetFont(&TextFont);
			SetFontFromOptions(TextFont, caps, lastMe ? Options::OutTimestamp : Options::InTimestamp);
			TextFont->SetBackColor(backColor);
			TextSelection->SetText(pStr);
			TextFont->Release();

			if (isUser)
			{
				if (lastMe)
					mir_sntprintf( str, MAXSELECTSTR, _T("%s\n"), myName );
				else
					mir_sntprintf( str, MAXSELECTSTR, _T("%s\n"), contactName );
				*strLen = (unsigned int)_tcslen(str) * sizeof(TCHAR);
				TextSelection->SetStart(MAXLONG);
				TextSelection->GetFont(&TextFont);
				SetFontFromOptions(TextFont, caps, lastMe ? Options::OutName : Options::InName);
				TextSelection->SetText(pStr);
				TextFont->Release();
			}
				
			GetEventMessage(hDbEvent, str);
			strStl = str;
			size_t i = strStl.length();
			if (i + 1 >= MAXSELECTSTR)
				continue;
			str[i++] = _T('\n');
			str[i] = 0;
			*strLen = (unsigned int)i * sizeof(TCHAR);
			TextSelection->SetStart(MAXLONG);
			TextSelection->GetFont(&TextFont);
			SetFontFromOptions(TextFont, caps, lastMe ? Options::OutMessages : Options::InMessages);
			TextFont->SetBackColor(backColor);
			TextSelection->GetStart(&startAt);
			TextSelection->SetText(pStr);
			TextFont->Release();

			if (Options::instance->messagesUseSmileys)
				ReplaceIcons(editWindow, startAt, lastMe);
			TextSelection->SetStart(MAXLONG);
			TextSelection->GetStart(&endAt);
			currentGroup.push_back(MessageData(strStl, startAt, endAt, lastMe, data.timestamp));
		}
	}
	
	TextSelection->SetRange(0, 0);
	TextSelection->Release();
	TextDocument->Unfreeze(&cnt);
	TextDocument->Release();
	RichEditOle->Release();
	SendMessage(editWindow, EM_SETREADONLY, TRUE, 0);
	SendMessage(editWindow,EM_SETBKGNDCOLOR,0, backColor);
	if (cnt == 0) 
	{
		UpdateWindow(editWindow);
	}

	if (isStartSelect && !Options::instance->messagesNewOnTop)
	{
		HWND h = SetFocus(editWindow);
		CHARRANGE ch;
		ch.cpMin = ch.cpMax = MAXLONG;
		SendMessage(editWindow,EM_EXSETSEL,0,(LPARAM)&ch);
		SendMessage(editWindow,EM_SCROLLCARET,0,0);
	}

	isStartSelect = false;
}

LRESULT CALLBACK HistoryWindow::SplitterSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndParent = GetParent(hwnd);
	HistoryWindow *dat = (HistoryWindow*)GetWindowLongPtr(hwndParent, GWLP_USERDATA);
	if (dat == NULL || dat->isDestroyed)
		return FALSE;

	switch (msg) {
		case WM_NCHITTEST:
			return HTCLIENT;
		case WM_SETCURSOR: {
			RECT rc;
			GetClientRect(hwnd, &rc);
			SetCursor(rc.right > rc.bottom ? hCurSplitNS : hCurSplitWE);
			return TRUE;
		}
		case WM_LBUTTONDOWN: {
			SetCapture(hwnd);
			return 0;
		}
		case WM_MOUSEMOVE:
			if (GetCapture() == hwnd) {
				RECT rc;
				GetClientRect(hwnd, &rc);
				SendMessage(hwndParent, DM_SPLITTERMOVED, rc.right > rc.bottom ? (short) HIWORD(GetMessagePos()) + rc.bottom / 2 : (short) LOWORD(GetMessagePos()) + rc.right / 2, (LPARAM) hwnd);
			}
			return 0;
		case WM_ERASEBKGND:
			return(1);
		case WM_LBUTTONUP: {
			HWND hwndCapture = GetCapture();

			ReleaseCapture();
			SendMessage(hwndParent, WM_SIZE, 0, 0);
			RedrawWindow(hwndParent, NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW);
			return 0;
		}
	}
	return mir_callNextSubclass(hwnd, HistoryWindow::SplitterSubclassProc, msg, wParam, lParam);
}

void HistoryWindow::EnableWindows(BOOL enable)
{
	EnableWindow(GetDlgItem(hWnd,IDC_LIST_CONTACTS), enable);
	EnableWindow(listWindow, enable);
	EnableWindow(findWindow, enable);
	EnableWindow(toolbarWindow, enable);
	isLoading = !enable;
}

void HistoryWindow::ReloadContacts()
{
	HWND contactList = GetDlgItem(hWnd,IDC_LIST_CONTACTS);
	if (EventList::GetContactMessageNumber(NULL)) {
		if (hSystem == NULL) {
			CLCINFOITEM cii = { sizeof(cii) };
			cii.flags = CLCIIF_GROUPFONT | CLCIIF_BELOWCONTACTS;
			cii.pszText = TranslateT("System");
			hSystem = (MCONTACT)SendMessage(contactList, CLM_ADDINFOITEM, 0, (LPARAM) & cii);
		}
	}
	else {
		if (hSystem != NULL) {
			SendMessage(contactList, CLM_DELETEITEM, (WPARAM)hSystem, 0);
			hSystem = NULL;
		}
	}

	for (MCONTACT _hContact = db_find_first(); _hContact; _hContact = db_find_next(_hContact)) {
		if (EventList::GetContactMessageNumber(_hContact) && (metaContactProto == NULL || !db_mc_isSub(_hContact))) {
			HANDLE hItem = (HANDLE)SendMessage(contactList, CLM_FINDCONTACT, (WPARAM)_hContact, 0);
			if (hItem == NULL)
				SendMessage(contactList, CLM_ADDCONTACT, (WPARAM)_hContact, 0);
		}
		else {
			HANDLE hItem = (HANDLE)SendMessage(contactList, CLM_FINDCONTACT, (WPARAM)_hContact, 0);
			if (hItem != NULL)
				SendMessage(contactList, CLM_DELETEITEM, (WPARAM)_hContact, 0);
		}
	}

	if (hContact != NULL) {
		HANDLE hItem = (HANDLE)SendMessage(contactList, CLM_FINDCONTACT, hContact, 0);
		if (hItem != NULL) {
			SendMessage(contactList, CLM_ENSUREVISIBLE, (WPARAM)hItem, 0);
			SendMessage(contactList, CLM_SELECTITEM, (WPARAM)hItem, 0);
		}
	}
	else if (hSystem != NULL) {
		SendMessage(contactList, CLM_ENSUREVISIBLE, (WPARAM)hSystem, 0);
		SendMessage(contactList, CLM_SELECTITEM, (WPARAM)hSystem, 0);
	}
}

bool HistoryWindow::DoHotkey(UINT msg, LPARAM lParam, WPARAM wParam, int window)
{
	MSG		message;
	message.hwnd = hWnd;
	message.message = msg;
	message.lParam = lParam;
	message.wParam = wParam;
	LRESULT mim_hotkey_check = CallService(MS_HOTKEY_CHECK, (WPARAM)&message, (LPARAM)("History"));
	switch(mim_hotkey_check)
	{
	case HISTORY_HK_FIND:
		SetFocus(findWindow);
		Edit_SetSel(findWindow, 0, -1);
		break;
	case HISTORY_HK_FINDNEXT:
		searcher.ChangeFindDirection(false);
		break;
	case HISTORY_HK_FINDPREV:
		searcher.ChangeFindDirection(true);
		break;
	case HISTORY_HK_MATCHCASE:
		searcher.SetMatchCase(!searcher.IsMatchCase());
		break;
	case HISTORY_HK_MATCHWHOLE:
		searcher.SetMatchWholeWords(!searcher.IsMatchWholeWords());
		break;
	case HISTORY_HK_SHOWCONTACTS:
		Button_SetCheck(GetDlgItem(hWnd, IDC_SHOWHIDE), Button_GetCheck(GetDlgItem(hWnd, IDC_SHOWHIDE)) & BST_CHECKED ? BST_UNCHECKED : BST_CHECKED);
        SendMessage(hWnd, WM_COMMAND, MAKELONG(IDC_SHOWHIDE, BN_CLICKED), NULL);
		break;
	case HISTORY_HK_ONLYIN:
		searcher.SetOnlyIn(!searcher.IsOnlyIn());
		break;
	case HISTORY_HK_ONLYOUT:
		searcher.SetOnlyOut(!searcher.IsOnlyOut());
		break;
	case HISTORY_HK_ONLYGROUP:
		searcher.SetOnlyGroup(!searcher.IsOnlyGroup());
		break;
	case HISTORY_HK_ALLCONTACTS:
		searcher.SetAllUsers(!searcher.IsAllUsers());
		break;
	case HISTORY_HK_EXRHTML:
		{
			ExportManager exp(hWnd, hContact, GetFilterNr());
			exp.Export(IExport::RichHtml);
		}
		break;
	case HISTORY_HK_EXPHTML:
		{
			ExportManager exp(hWnd, hContact, GetFilterNr());
			exp.Export(IExport::PlainHtml);
		}
		break;
	case HISTORY_HK_EXTXT:
		{
			ExportManager exp(hWnd, hContact, GetFilterNr());
			exp.Export(IExport::Txt);
		}
		break;
	case HISTORY_HK_EXBIN:
		{
			ExportManager exp(hWnd, hContact, GetFilterNr());
			exp.Export(IExport::Binary);
		}
		break;
	case HISTORY_HK_EXDAT:
		{
			ExportManager exp(hWnd, hContact, GetFilterNr());
			exp.Export(IExport::Dat);
		}
		break;
	case HISTORY_HK_IMPBIN:
		{
			DoImport(IImport::Binary);
		}
		break;
	case HISTORY_HK_IMPDAT:
		{
			DoImport(IImport::Dat);
		}
		break;
	case HISTORY_HK_DELETE:
		{
			int what = window == IDC_EDIT ? 0 : (window == IDC_LIST ? 1 : (window == IDC_LIST_CONTACTS ? 2 : -1));
			Delete(what);
			return what != -1;
		}
		break;
	default:
		return false;
	}

	return true;
}

void HistoryWindow::RestorePos()
{
	MCONTACT contactToLoad = hContact;
	if (hContact == NULL) {
		Utils_RestoreWindowPosition(hWnd,NULL,MODULE,"history_");
		contactToLoad = NULL;
	}
	else if (Utils_RestoreWindowPosition(hWnd,hContact,MODULE,"history_") != 0) {
		Utils_RestoreWindowPosition(hWnd,NULL,MODULE,"history_");
		contactToLoad = NULL;
	}

	if (db_get_b(contactToLoad, MODULE, "history_ismax", 0))
		ShowWindow(hWnd, SW_SHOWMAXIMIZED);

	LONG pos = db_get_dw(contactToLoad, MODULE, "history_splitterv", 0);
	if (pos > 0)
		SplitterMoved(splitterYhWnd, pos, false);

	pos = db_get_dw(contactToLoad, MODULE, "history_splitter", 0);
	if (pos > 0)
		SplitterMoved(splitterXhWnd, pos, false);
}

void HistoryWindow::SavePos(bool all)
{
	MCONTACT contactToSave = hContact;
	if (all) {
		for (MCONTACT _hContact = db_find_first(); _hContact; _hContact = db_find_next(_hContact)) {
			db_unset(_hContact, MODULE, "history_x");
			db_unset(_hContact, MODULE, "history_y");
			db_unset(_hContact, MODULE, "history_width");
			db_unset(_hContact, MODULE, "history_height");
			db_unset(_hContact, MODULE, "history_ismax");
			db_unset(_hContact, MODULE, "history_splitterv");
			db_unset(_hContact, MODULE, "history_splitter");
		}

		contactToSave = NULL;
	}
	
	Utils_SaveWindowPosition(hWnd,contactToSave,MODULE,"history_");
	WINDOWPLACEMENT wp;
	wp.length=sizeof(wp);
	GetWindowPlacement(hWnd,&wp);
	db_set_b(contactToSave, MODULE, "history_ismax", wp.showCmd == SW_MAXIMIZE ? 1 : 0);
	db_set_dw(contactToSave, MODULE, "history_splitterv", splitterX);
	db_set_dw(contactToSave, MODULE, "history_splitter", splitterY);
}

#define DEF_FILTERS_START 50000
void HistoryWindow::FindToolbarClicked(LPNMTOOLBAR lpnmTB)
{
	RECT rc;
	SendMessage(lpnmTB->hdr.hwndFrom, TB_GETRECT, (WPARAM)lpnmTB->iItem, (LPARAM)&rc); 
	MapWindowPoints(lpnmTB->hdr.hwndFrom, HWND_DESKTOP, (LPPOINT)&rc, 2);         
	HMENU hPopupMenu = CreatePopupMenu();
	if (hPopupMenu != NULL)
	{
		AppendMenu(hPopupMenu, MF_STRING, IDM_FINDNEXT, TranslateT("Find Next"));
		AppendMenu(hPopupMenu, MF_STRING, IDM_FINDPREV, TranslateT("Find Previous"));
		AppendMenu(hPopupMenu, MFT_SEPARATOR, 0, NULL);
		AppendMenu(hPopupMenu, searcher.IsMatchCase() ? MF_STRING | MF_CHECKED : MF_STRING, IDM_MATCHCASE, TranslateT("Match Case"));
		AppendMenu(hPopupMenu, searcher.IsMatchWholeWords() ? MF_STRING | MF_CHECKED : MF_STRING, IDM_MATCHWHOLE, TranslateT("Match Whole Word"));
		AppendMenu(hPopupMenu, searcher.IsOnlyIn() ? MF_STRING | MF_CHECKED : MF_STRING, IDM_ONLYIN, TranslateT("Only Incoming Messages"));
		AppendMenu(hPopupMenu, searcher.IsOnlyOut() ? MF_STRING | MF_CHECKED : MF_STRING, IDM_ONLYOUT, TranslateT("Only Outgoing Messages"));
		AppendMenu(hPopupMenu, searcher.IsOnlyGroup() ? MF_STRING | MF_CHECKED : MF_STRING, IDM_ONLYGROUP, TranslateT("Only Selected Group"));
		AppendMenu(hPopupMenu, searcher.IsAllUsers() ? MF_STRING | MF_CHECKED : MF_STRING, IDM_ALLUSERS, TranslateT("All Contacts"));
		AppendMenu(hPopupMenu, MFT_SEPARATOR, 0, NULL);
		HMENU hFilterMenu = CreatePopupMenu();
		int filter = GetFilterNr();
		AppendMenu(hFilterMenu, filter == 0 ? MF_STRING | MF_CHECKED : MF_STRING, IDM_FILTERDEF, TranslateT("Default history events"));
		AppendMenu(hFilterMenu, filter == 1 ? MF_STRING | MF_CHECKED : MF_STRING, IDM_FILTERALL, TranslateT("All events"));
		for (size_t i = 0 ; i < Options::instance->customFilters.size(); ++i)
		{
			UINT flags = MF_STRING;
			if (filter - 2 == i)
				flags |= MF_CHECKED;

			AppendMenu(hFilterMenu, flags, DEF_FILTERS_START + i, Options::instance->customFilters[i].name.c_str());
		}
		AppendMenu(hPopupMenu, MF_STRING | MF_POPUP, (UINT_PTR)hFilterMenu, TranslateT("Filters"));
		if (searcher.IsFindBack())
			SetMenuDefaultItem(hPopupMenu, IDM_FINDPREV, FALSE);
		else
			SetMenuDefaultItem(hPopupMenu, IDM_FINDNEXT, FALSE);

		int selected = TrackPopupMenu(hPopupMenu, TPM_RETURNCMD, rc.left, rc.bottom, 0, hWnd, 0);
		switch (selected)
		{
		case IDM_FINDNEXT:
			searcher.ChangeFindDirection(false);
			break;
		case IDM_FINDPREV:
			searcher.ChangeFindDirection(true);
			break;
		case IDM_MATCHCASE:
			searcher.SetMatchCase(!searcher.IsMatchCase());
			break;
		case IDM_MATCHWHOLE:
			searcher.SetMatchWholeWords(!searcher.IsMatchWholeWords());
			break;
		case IDM_ONLYIN:
			searcher.SetOnlyIn(!searcher.IsOnlyIn());
			break;
		case IDM_ONLYOUT:
			searcher.SetOnlyOut(!searcher.IsOnlyOut());
			break;
		case IDM_ONLYGROUP:
			searcher.SetOnlyGroup(!searcher.IsOnlyGroup());
			break;
		case IDM_ALLUSERS:
			searcher.SetAllUsers(!searcher.IsAllUsers());
			break;
		case IDM_FILTERDEF:
			SetDefFilter(0);
			SendMessage(hWnd,DM_HREBUILD,0,0);
			break;
		case IDM_FILTERALL:
			SetDefFilter(1);
			SendMessage(hWnd,DM_HREBUILD,0,0);
			break;
		default:
			if (selected >= DEF_FILTERS_START)
			{
				SetDefFilter(selected - DEF_FILTERS_START + 2);
				SendMessage(hWnd,DM_HREBUILD,0,0);
			}
			break;
		}
		
		DestroyMenu(hFilterMenu);
		DestroyMenu(hPopupMenu);
	}
}

void HistoryWindow::ConfigToolbarClicked(LPNMTOOLBAR lpnmTB)
{
	RECT rc;
	SendMessage(lpnmTB->hdr.hwndFrom, TB_GETRECT, (WPARAM)lpnmTB->iItem, (LPARAM)&rc); 
	MapWindowPoints(lpnmTB->hdr.hwndFrom, HWND_DESKTOP, (LPPOINT)&rc, 2);         
	HMENU hPopupMenu = CreatePopupMenu();
	if (hPopupMenu != NULL)
	{
		AppendMenu(hPopupMenu, MF_STRING, IDM_OPTIONS, TranslateT("Options"));
		AppendMenu(hPopupMenu, MF_STRING, IDM_FONTS, TranslateT("Fonts and colors"));
		AppendMenu(hPopupMenu, MF_STRING, IDM_ICONS, TranslateT("Icons"));
		AppendMenu(hPopupMenu, MF_STRING, IDM_HOTKEYS, TranslateT("Hotkeys"));
		AppendMenu(hPopupMenu, MFT_SEPARATOR, 0, NULL);
		
		HMENU hExportMenu = CreatePopupMenu();
		AppendMenu(hExportMenu, MF_STRING, IDM_EXPORTRHTML, TranslateT("Rich Html"));
		AppendMenu(hExportMenu, MF_STRING, IDM_EXPORTPHTML, TranslateT("Plain Html"));
		AppendMenu(hExportMenu, MF_STRING, IDM_EXPORTTXT, TranslateT("Txt"));
		AppendMenu(hExportMenu, MF_STRING, IDM_EXPORTBINARY, TranslateT("Binary"));
		AppendMenu(hExportMenu, MF_STRING, IDM_EXPORTDAT, TranslateT("Dat (mContacts)"));
		
		HMENU hImportMenu = CreatePopupMenu();
		AppendMenu(hImportMenu, MF_STRING, IDM_IMPORTBINARY, TranslateT("Binary"));
		AppendMenu(hImportMenu, MF_STRING, IDM_IMPORTDAT, TranslateT("Dat (mContacts)"));

		AppendMenu(hPopupMenu, MF_STRING | MF_POPUP, (UINT_PTR)hExportMenu, TranslateT("Export"));
		AppendMenu(hPopupMenu, MF_STRING | MF_POPUP, (UINT_PTR)hImportMenu, TranslateT("Import"));

		AppendMenu(hPopupMenu, MFT_SEPARATOR, 0, NULL);
		AppendMenu(hPopupMenu, MF_STRING, IDM_SAVEPOS, TranslateT("Save window position as default"));
		AppendMenu(hPopupMenu, MF_STRING, IDM_SAVEPOSALL, TranslateT("Save window position for all contacts"));
		SetMenuDefaultItem(hPopupMenu, IDM_OPTIONS, FALSE);

		int selected = TrackPopupMenu(hPopupMenu, TPM_RETURNCMD, rc.left, rc.bottom, 0, hWnd, 0);
		switch (selected)
		{
		case IDM_OPTIONS:
			SendMessage(hWnd, WM_COMMAND, IDM_CONFIG, 0);
			break;
		case IDM_FONTS:
			OpenOptions("Customize", "Fonts and colors");
			break;
		case IDM_ICONS:
			OpenOptions("Customize", "Icons");
			break;
		case IDM_HOTKEYS:
			OpenOptions("Customize", "Hotkeys");
			break;
		case IDM_SAVEPOS:
			SavePos(false);
			break;
		case IDM_SAVEPOSALL:
			SavePos(true);
			break;
		case IDM_EXPORTRHTML:
			{
				ExportManager exp(hWnd, hContact, GetFilterNr());
				exp.Export(IExport::RichHtml);
			}

			break;
		case IDM_EXPORTPHTML:
			{
				ExportManager exp(hWnd, hContact, GetFilterNr());
				exp.Export(IExport::PlainHtml);
			}

			break;
		case IDM_EXPORTTXT:
			{
				ExportManager exp(hWnd, hContact, GetFilterNr());
				exp.Export(IExport::Txt);
			}

			break;
		case IDM_EXPORTBINARY:
			{
				ExportManager exp(hWnd, hContact, GetFilterNr());
				exp.Export(IExport::Binary);
			}

			break;
		case IDM_EXPORTDAT:
			{
				ExportManager exp(hWnd, hContact, GetFilterNr());
				exp.Export(IExport::Dat);
			}

			break;
		case IDM_IMPORTBINARY:
			{
				DoImport(IImport::Binary);
			}

			break;
		case IDM_IMPORTDAT:
			{
				DoImport(IImport::Dat);
			}

			break;
		}
		
		DestroyMenu(hExportMenu);
		DestroyMenu(hImportMenu);
		DestroyMenu(hPopupMenu);
	}
}

void HistoryWindow::DoImport(IImport::ImportType type)
{
	ExportManager exp(hWnd, hContact, GetFilterNr());
	std::vector<IImport::ExternalMessage> messages;
	std::wstring err;
	std::vector<MCONTACT> contacts;

	for (MCONTACT _hContact = db_find_first(); _hContact != NULL; _hContact = db_find_next(_hContact))
		contacts.push_back(_hContact);

	bool changeContact = false;
	MCONTACT lastContact = hContact;
	int i = 1;
	do
	{
		bool differentContact = false;
		if (exp.Import(type, messages, &err, &differentContact, &contacts)) {
			int act = MessageBox(hWnd, TranslateT("Do you want to save imported messages to local profile?"), TranslateT("Import"), MB_ICONQUESTION | MB_YESNOCANCEL | MB_DEFBUTTON2);
			if (act == IDYES) {
				MargeMessages(messages);
				if (!changeContact)
					HistoryWindow::RebuildEvents(hContact);
			}
			else if (act == IDNO) {
				EventList::AddImporter(hContact, type, exp.GetFileName());
				if (!changeContact)
					HistoryWindow::RebuildEvents(hContact);
			}
		}
		else if (differentContact)
		{
			int act = MessageBox(hWnd, TranslateT("File contains history for different contact. Do you want to change contact and import?"), TranslateT("Error"), MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2);
			if (act == IDYES) {
				changeContact = true;
				hContact = exp.hContact;
			}
		}
		else if (!err.empty())
		{
			MessageBox(hWnd, err.c_str(), TranslateT("Error"), MB_ICONERROR);
		}
	}
		while(changeContact && i--);
	
	if (changeContact) {
		hContact = lastContact;
		ReloadContacts();
		SelectContact(exp.hContact);
		HistoryWindow::RebuildEvents(exp.hContact);
	}
}

void HistoryWindow::DeleteToolbarClicked(LPNMTOOLBAR lpnmTB)
{
	RECT rc;
	SendMessage(lpnmTB->hdr.hwndFrom, TB_GETRECT, (WPARAM)lpnmTB->iItem, (LPARAM)&rc); 
	MapWindowPoints(lpnmTB->hdr.hwndFrom, HWND_DESKTOP, (LPPOINT)&rc, 2);         
	HMENU hPopupMenu = CreatePopupMenu();
	if (hPopupMenu != NULL) {
		AppendMenu(hPopupMenu, MF_STRING, IDM_DELETE, TranslateT("Delete"));
		AppendMenu(hPopupMenu, MF_STRING, IDM_DELETEGROUP, TranslateT("Delete Group"));
		AppendMenu(hPopupMenu, MF_STRING, IDM_DELETEUSER, TranslateT("Delete All User History"));
		SetMenuDefaultItem(hPopupMenu, IDM_DELETE, FALSE);

		int selected = TrackPopupMenu(hPopupMenu, TPM_RETURNCMD, rc.left, rc.bottom, 0, hWnd, 0);
		switch (selected) {
		case IDM_DELETE:
			Delete(0);
			break;
		case IDM_DELETEGROUP:
			Delete(1);
			break;
		case IDM_DELETEUSER:
			Delete(2);
			break;
		}

		DestroyMenu(hPopupMenu);
	}
}

void HistoryWindow::Delete(int what)
{
	int toDelete = 1;
	size_t start = 0;
	size_t end = 0;
	if (selected < 0 || selected >= (int)eventList.size() || what > 2 || what < 0)
		return;
	if (what == 0) {
		CHARRANGE chrg;
		SendMessage(editWindow,EM_EXGETSEL,0,(LPARAM)&chrg);
		if (chrg.cpMin == 0 && chrg.cpMax == -1)
			toDelete = (int)currentGroup.size();
		else {
			while(start < currentGroup.size() && chrg.cpMin >= currentGroup[start].endPos) ++start;
			end = start;
			while(end < currentGroup.size() && chrg.cpMax > currentGroup[end].endPos) ++end;
			if (start >= currentGroup.size())
				return;
			if (end < currentGroup.size())
				++end;
			toDelete = (int)(end - start);
		}
	}
	else if (what == 1) {
		end = currentGroup.size();
		toDelete = (int)end;
	}
	else {
		DeleteAllUserHistory(hContact, 0);
		return;
	}

	if (toDelete == 0)
		return;

	TCHAR message[256];
	mir_sntprintf(message, SIZEOF(message), TranslateT("Number of history items to delete: %d.\nAre you sure you want to do this?"), toDelete);
	if (MessageBox(hWnd, message, TranslateT("Are You sure?"), MB_OKCANCEL | MB_ICONERROR) != IDOK)
		return;

	bool areImpMessages = false;
	bool rebuild = false;
	for (size_t i = start; i < end; ++i) {
		EventIndex& ev = eventList[selected][i];
		DeleteEvent(ev);
		areImpMessages |= ev.isExternal;
	}

	rebuild = (start == 0 && end == currentGroup.size());

	if (areImpMessages) {
		TCHAR *message = TranslateT("Do you want to delete all imported messages for this contact?\nNote that next scheduler task import this messages again.");
		if (MessageBox(hWnd, message, TranslateT("Are You sure?"), MB_YESNO | MB_ICONERROR) == IDYES) {
			EventList::DeleteImporter(hContact);
			rebuild = true;
		}
	}

	if (rebuild)
		RebuildEvents(hContact);
	else {
		RebuildGroup(selected);
		SelectEventGroup(selected);
	}
}

bool HistoryWindow::ContactChanged(bool sync)
{
	if (!isLoading)
	{
		MCONTACT hItem = (MCONTACT)SendDlgItemMessage(hWnd, IDC_LIST_CONTACTS, CLM_GETSELECTION, 0, 0);
		if (hItem != NULL) {
			int typeOf = SendDlgItemMessage(hWnd, IDC_LIST_CONTACTS, CLM_GETITEMTYPE,(WPARAM)hItem,0);
			if (typeOf == CLCIT_CONTACT) {
				if (hContact != hItem) {
					ChangeToFreeWindow(this);
					isLoading = true;
					hContact = hItem;
					ReloadContacts();
					if (sync)
						FillHistoryThread(this);
					else
						mir_forkthread(HistoryWindow::FillHistoryThread, this);
					return true;
				}
			}
			else if (typeOf == CLCIT_INFO && hSystem == hItem) {
				if (hContact != NULL) {
					ChangeToFreeWindow(this);
					isLoading = true;
					hContact = NULL;
					ReloadContacts();
					if (sync)
						FillHistoryThread(this);
					else
						mir_forkthread(HistoryWindow::FillHistoryThread, this);
					return true;
				}
			}
		}
	}

	return false;
}

void HistoryWindow::GroupImagesChanged()
{
	if (isGroupImages != Options::instance->groupShowEvents) {
		isGroupImages = Options::instance->groupShowEvents;
		if (isGroupImages)
			ListView_SetImageList(listWindow, himlSmall, LVSIL_SMALL);
		else
			ListView_SetImageList(listWindow, himlNone, LVSIL_SMALL);
	}
}

void HistoryWindow::FormatQuote(std::wstring& quote, const MessageData& md, const std::wstring& msg)
{
	if (md.isMe)
		quote += myName;
	else
		quote += contactName;
	TCHAR str[32];
	tmi.printTimeStamp(NULL, md.timestamp, _T("d t"), str , 32, 0);
	quote += _T(", ");
	quote += str;
	quote += _T("\n");
	int f = 0;
	do
	{
		int nf = (int)msg.find_first_of(_T("\r\n"), f);
		if (nf >= 0 && nf < (int)msg.length()) {
			if (nf - f >= 0) {
				quote += _T(">");
				quote += msg.substr(f, nf - f);
				quote += _T("\n");
			}

			f = nf + 1;
			if (msg[nf] == _T('\r') && f < (int)msg.length() && msg[f] == _T('\n'))
				++f;
		}
		else if (msg.length() - f > 0) {
			quote += _T(">");
			quote += msg.substr(f, msg.length() - f);
			quote += _T("\n");
			f = -1;
		}
		else f = -1;
	}
		while(f > 0 && f < (int)msg.length());
}

MCONTACT HistoryWindow::GetNextContact(MCONTACT hContact, int adder)
{
	HWND contactList = GetDlgItem(hWnd,IDC_LIST_CONTACTS);
	bool find = false;
	MCONTACT _hContact;
	if (adder > 0) {
		if (hContact != NULL) {
			_hContact = db_find_next(hContact);
			while(_hContact) {
				HANDLE hItem = (HANDLE)SendMessage(contactList, CLM_FINDCONTACT, (WPARAM)_hContact, 0);
				if (hItem != NULL) {
					find = true;
					break;
				}
			
				_hContact = db_find_next(_hContact);
			}
		
			if (!find && EventList::GetContactMessageNumber(NULL)) {
				_hContact = NULL;
				find = true;
			}
		}

		if (!find) {
			_hContact = db_find_first();
			while(_hContact && _hContact != hContact) {
				HANDLE hItem = (HANDLE)SendMessage(contactList, CLM_FINDCONTACT, (WPARAM)_hContact, 0);
				if (hItem != NULL) {
					find = true;
					break;
				}
			
				_hContact = db_find_next(_hContact);
			}
		}
	}
	else {
		MCONTACT lastContact = NULL;
		_hContact = db_find_first();
		while(_hContact && _hContact != hContact) {
			HANDLE hItem = (HANDLE)SendMessage(contactList, CLM_FINDCONTACT, (WPARAM)_hContact, 0);
			if (hItem != NULL)
				lastContact = _hContact;
			
			_hContact = db_find_next(_hContact);
		}
		
		if (hContact != NULL) {
			if (lastContact == NULL && !EventList::GetContactMessageNumber(NULL)) {
				_hContact = db_find_next(hContact);
				while(_hContact) {
					HANDLE hItem = (HANDLE)SendMessage(contactList, CLM_FINDCONTACT, (WPARAM)_hContact, 0);
					if (hItem != NULL)
						lastContact = _hContact;
			
					_hContact = db_find_next(_hContact);
				}
			}

			if (lastContact != NULL || EventList::GetContactMessageNumber(NULL)) {
				_hContact = lastContact;
				find = true;
			}
		}
		else if (lastContact != NULL) {
			_hContact = lastContact;
			find = true;
		}
	}

	if (find)
		return _hContact;

	return hContact;
}

void HistoryWindow::SelectContact(MCONTACT _hContact)
{
	if (hContact != _hContact) {
		HWND contactList = GetDlgItem(hWnd,IDC_LIST_CONTACTS);
		if (_hContact != NULL) {
			HANDLE hItem = (HANDLE)SendMessage(contactList, CLM_FINDCONTACT, (WPARAM)_hContact, 0);
			if (hItem != NULL) {
				SendMessage(contactList, CLM_ENSUREVISIBLE, (WPARAM)hItem, 0);
				SendMessage(contactList, CLM_SELECTITEM, (WPARAM)hItem, 0);
			}
		}
		else {
			SendMessage(contactList, CLM_ENSUREVISIBLE, (WPARAM)hSystem, 0);
			SendMessage(contactList, CLM_SELECTITEM, (WPARAM)hSystem, 0);
		}

		while(isLoading)
			Sleep(100);
		ContactChanged(true);
	}
}

void ResetCList(HWND hWnd)
{
	COLORREF bkCLColor = Options::instance->GetColor(Options::ContactListBackground);
	SendDlgItemMessage(hWnd, IDC_LIST_CONTACTS, CLM_SETBKBITMAP, 0, (LPARAM)(HBITMAP) NULL);
	SendDlgItemMessage(hWnd, IDC_LIST_CONTACTS, CLM_SETBKCOLOR, bkCLColor, 0);
	SendDlgItemMessage(hWnd, IDC_LIST_CONTACTS, CLM_SETUSEGROUPS, Options::instance->showContactGroups, 0);
}
