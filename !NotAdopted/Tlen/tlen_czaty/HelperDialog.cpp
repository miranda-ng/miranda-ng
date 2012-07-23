/*

MUCC Group Chat GUI Plugin for Miranda IM
Copyright (C) 2004  Piotr Piastucki

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "HelperDialog.h"
#include "Utils.h"

static INT_PTR CALLBACK InviteDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK AcceptInvitationDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK JoinDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
//static INT_PTR CALLBACK ErrorDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK TopicDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static void __cdecl InviteDlgThread(void *vDialog);
static void __cdecl AcceptInvitationDlgThread(void *vDialog);
static void __cdecl JoinDlgThread(void *vDialog);
static void __cdecl ErrorDlgThread(void *vDialog);
static void __cdecl TopicDlgThread(void *vDialog);

HelperDialog *	HelperDialog::list = NULL;
CRITICAL_SECTION HelperDialog::mutex;

HelperDialog::HelperContact::HelperContact(const char *id, const char *name) {
	this->id = NULL;
	this->name = NULL;
	next = NULL;
	Utils::copyString(&this->id, id);
	Utils::copyString(&this->name, name);
}
HelperDialog::HelperContact::~HelperContact() {
	if (id != NULL) delete id;
	if (name != NULL) delete name;
}

void HelperDialog::HelperContact::setNext(HelperContact *next) {
	this->next = next;
}

const char *HelperDialog::HelperContact::getId() {
	return id;
}

const char *HelperDialog::HelperContact::getName() {
	return name;
}

HelperDialog::HelperContact* HelperDialog::HelperContact::getNext() {
	return next;
}

void HelperDialog::init() {
	InitializeCriticalSection(&mutex);
}

void HelperDialog::release() {
	while (list!=NULL) {
		delete list;
	}
	DeleteCriticalSection(&mutex);
}

HelperDialog::HelperDialog()  {
	module = nick = reason = roomId = roomName = NULL;
	prev = next = NULL;
	contactList = NULL;
	hWnd = NULL;
	hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	EnterCriticalSection(&mutex);
	setNext(list);
	if (next!=NULL) {
		next->setPrev(this);
	}
	list = this;
	LeaveCriticalSection(&mutex);
}

HelperDialog::HelperDialog(ChatWindow *chat) {
	module = nick = reason = roomId = roomName = NULL;
	prev = next = NULL;
	contactList = NULL;
	hWnd = NULL;
	hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	setModule(chat->getModule());
	setRoomId(chat->getRoomId());
	setRoomName(chat->getRoomName());
	setRoomFlags(chat->getRoomFlags());
	EnterCriticalSection(&mutex);
	setNext(list);
	if (next!=NULL) {
		next->setPrev(this);
	}
	list = this;
	LeaveCriticalSection(&mutex);
}

HelperDialog::~HelperDialog() 
{
	EnterCriticalSection(&mutex);
	if (getPrev()!=NULL) {
		getPrev()->setNext(next);
	} else {
		list = getNext();
	}
	if (getNext()!=NULL) {
		getNext()->setPrev(prev);
	}
	LeaveCriticalSection(&mutex);
	if (hEvent!=NULL) CloseHandle(hEvent);
	if (module!=NULL) delete module;
	if (roomId!=NULL) delete roomId;
	if (roomName!=NULL) delete roomName;
	if (nick!=NULL) delete nick;
	if (reason!=NULL) delete reason;
	for (HelperContact *ptr2, *ptr=contactList;ptr!=NULL;ptr=ptr2) {
		ptr2 =  ptr->getNext();
		delete ptr;
	}
}
void HelperDialog::setHWND(HWND hWnd) {
	this->hWnd = hWnd;
}
HWND HelperDialog::getHWND() {
	return hWnd;
}
void HelperDialog::setPrev(HelperDialog *prev) {
	this->prev = prev;
}
void HelperDialog::setNext(HelperDialog *next) {
	this->next = next;
}
HelperDialog * HelperDialog::getPrev() {
	return prev;
}
HelperDialog * HelperDialog::getNext() {
	return next;
}
const char *HelperDialog::getModule() 
{
	return module;
}
const char *HelperDialog::getRoomId() 
{
	return roomId;
}
const char *HelperDialog::getRoomName() 
{
	return roomName;
}
const char *HelperDialog::getNick() 
{
	return nick;
}
const char *HelperDialog::getReason() 
{
	return reason;
}
void HelperDialog::setModule(const char *module)
{
	Utils::copyString(&this->module, module);
}
void HelperDialog::setRoomId(const char *roomId)
{
	Utils::copyString(&this->roomId, roomId);
}
void HelperDialog::setRoomName(const char *roomName)
{
	Utils::copyString(&this->roomName, roomName);
}
void HelperDialog::setNick(const char *nick)
{
	Utils::copyString(&this->nick, nick);
}
void HelperDialog::setReason(const char *reason)
{
	Utils::copyString(&this->reason, reason);
}
void HelperDialog::setRoomFlags(DWORD flags)
{
	this->roomFlags = flags;
}
void HelperDialog::setContactList(MUCCQUERYRESULT *queryResult) {
	HelperContact *ptr, *lastptr;
	for (ptr=contactList;ptr!=NULL;ptr=ptr->getNext()) {
		delete ptr;
	}
	contactList = NULL;
	lastptr = NULL;
	for (int i=0;i<queryResult->iItemsNum;i++) {
		ptr = new HelperContact(queryResult->pItems[i].pszID, queryResult->pItems[i].pszName);
		if (lastptr != NULL) {
			lastptr->setNext(ptr);
		} else {
			contactList = ptr;
		}
		lastptr = ptr;
	}
}

HelperDialog::HelperContact * HelperDialog::getContactList() {
	return contactList;
}

DWORD HelperDialog::getRoomFlags() {
	return roomFlags;
}

void HelperDialog::inviteDlg(ChatWindow *chat, MUCCQUERYRESULT *queryResult) {
	HelperDialog *dialog=new HelperDialog(chat);
	dialog->setContactList(queryResult);
	Utils::forkThread((void (__cdecl *)(void *))InviteDlgThread, 0, (void *) dialog);
	//WaitForSingleObject(dialog->getEvent(), INFINITE);
}

void HelperDialog::acceptDlg(MUCCEVENT *event) {
	HelperDialog *dialog=new HelperDialog();
	dialog->setModule(event->pszModule);
	dialog->setRoomId(event->pszID);
	dialog->setRoomName(event->pszName);
	dialog->setRoomFlags(event->dwFlags);
	dialog->setNick(event->pszNick);
	//	dialog->setReason(event->pszText);
	Utils::forkThread((void (__cdecl *)(void *))AcceptInvitationDlgThread, 0, (void *) dialog);
//	WaitForSingleObject(hEvent, INFINITE);
}

void HelperDialog::joinDlg(MUCCEVENT *event) {
	HelperDialog *dialog=new HelperDialog();
	dialog->setModule(event->pszModule);
	dialog->setRoomId(event->pszID);
	dialog->setRoomName(event->pszName);
	dialog->setRoomFlags(event->dwFlags);
	dialog->setNick(event->pszNick);
	Utils::forkThread((void (__cdecl *)(void *))JoinDlgThread, 0, (void *) dialog);
	//WaitForSingleObject(dialog->getEvent(), INFINITE);
}

void HelperDialog::errorDlg(MUCCEVENT *event) {
	HelperDialog *dialog=new HelperDialog();
	dialog->setReason(event->pszText);
	Utils::forkThread((void (__cdecl *)(void *))ErrorDlgThread, 0, (void *) dialog);
	//WaitForSingleObject(dialog->getEvent(), INFINITE);
}

void HelperDialog::topicDlg(ChatWindow *chat)
{
	HelperDialog *dialog=new HelperDialog();
	dialog->setModule(chat->getModule());
	dialog->setRoomId(chat->getRoomId());
	Utils::forkThread((void (__cdecl *)(void *))TopicDlgThread, 0, (void *) dialog);
}

static void __cdecl InviteDlgThread(void *vDialog)
{
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_HELPER_INVITE), NULL, InviteDlgProc, (LPARAM) vDialog);
}

static void __cdecl AcceptInvitationDlgThread(void *vDialog)
{
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_HELPER_INVITE_ACCEPT), NULL, AcceptInvitationDlgProc, (LPARAM) vDialog);
}

static void __cdecl JoinDlgThread(void *vDialog)
{
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_HELPER_JOIN), NULL, JoinDlgProc, (LPARAM) vDialog);
}

static void __cdecl TopicDlgThread(void *vDialog)
{
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_HELPER_TOPIC), NULL, TopicDlgProc, (LPARAM) vDialog);
}

static INT_PTR CALLBACK InviteDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{	
	char str[256];
	HelperDialog::HelperContact *contactList;
	HelperDialog *dialog = (HelperDialog *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch (msg) {
	case WM_INITDIALOG:
		dialog = (HelperDialog *) lParam;
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) dialog);
		dialog->setHWND(hwndDlg);
		SendDlgItemMessage(hwndDlg, IDC_REASON, EM_SETREADONLY, (WPARAM)TRUE, 0);
		for (contactList = dialog->getContactList();contactList!=NULL;contactList=contactList->getNext()) {
			SendDlgItemMessage(hwndDlg, IDC_USER, CB_ADDSTRING, 0, (LPARAM)contactList->getName());
		}
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_INVITE:
			MUCCEVENT muce;
			GetDlgItemTextA(hwndDlg, IDC_USER, str, 255);
			if (strlen(str)>0) {
				for (contactList = dialog->getContactList();contactList!=NULL;contactList=contactList->getNext()) {
					if (!strcmp(str, contactList->getName())) {
						dialog->setNick(contactList->getId());
						break;
					}
				}
				if (contactList==NULL) {
					dialog->setNick(str);
				}
			}
			muce.iType = MUCC_EVENT_INVITE;
			muce.pszModule = dialog->getModule();
			muce.pszID = dialog->getRoomId();
			muce.pszNick = dialog->getNick();
			NotifyEventHooks(hHookEvent, 0,(WPARAM)&muce);
		case IDCANCEL:
		case IDCLOSE:
			EndDialog(hwndDlg, 0);
			return TRUE;
		}
		break;
	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;
	case WM_DESTROY:
		delete dialog;
		break;
	}
	return FALSE;
}




static INT_PTR CALLBACK AcceptInvitationDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{	
	char str[256];
	HelperDialog *dialog;
	dialog = (HelperDialog *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch (msg) {
	case WM_INITDIALOG:
		{
			dialog = (HelperDialog *) lParam;
			TranslateDialogDefault(hwndDlg);
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) dialog);
			dialog->setHWND(hwndDlg);
			if (dialog->getNick() != NULL) {
				SetDlgItemTextA(hwndDlg, IDC_FROM, dialog->getNick());
			}
			if (dialog->getRoomName() != NULL) {
				SetDlgItemTextA(hwndDlg, IDC_ROOM, dialog->getRoomName());
			}
			if (dialog->getReason() != NULL) {
				SetDlgItemTextA(hwndDlg, IDC_REASON, dialog->getReason());
			}

//			if (!DBGetContactSetting(NULL, jabberProtoName, "LoginName", &dbv)) {
//				SetDlgItemText(hwndDlg, IDC_NICK, dbv.pszVal);
//				DBFreeVariant(&dbv);
//			}

			/*
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM) LoadIcon(hInst, MAKEINTRESOURCE(IDI_GROUP)));
*/
		}
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_ACCEPT:
			GetDlgItemTextA(hwndDlg, IDC_NICK, str, 255);
			dialog->setNick(str);
			MUCCEVENT muce;
			muce.iType = MUCC_EVENT_JOIN;
			muce.pszModule = dialog->getModule();
			muce.pszID = dialog->getRoomId();
			muce.pszName = dialog->getRoomName();
			muce.dwFlags = dialog->getRoomFlags();
			muce.pszNick = NULL;
			if (strlen(dialog->getNick())>0) {
				muce.pszNick = dialog->getNick();
			} else {
				muce.pszNick = NULL;
			}
			NotifyEventHooks(hHookEvent, 0,(WPARAM)&muce);
		case IDCANCEL:
		case IDCLOSE:
			EndDialog(hwndDlg, 0);
			return TRUE;
		}
		break;
	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;
	case WM_DESTROY:
		delete dialog;
		break;
	}

	return FALSE;
}


static INT_PTR CALLBACK JoinDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{	
	char str[256];
	HelperDialog *dialog;
	dialog = (HelperDialog *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch (msg) {
	case WM_INITDIALOG:
		dialog = (HelperDialog *) lParam;
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) dialog);
		dialog->setHWND(hwndDlg);
		SetFocus(GetDlgItem(hwndDlg, IDOK));
		if (dialog->getRoomFlags() & MUCC_EF_ROOM_NAME) {
			SetFocus(GetDlgItem(hwndDlg, IDC_ROOM));
		} else {
			SendDlgItemMessage(hwndDlg, IDC_ROOM, EM_SETREADONLY, (WPARAM)TRUE, 0);
		}
		if (dialog->getRoomFlags() & MUCC_EF_ROOM_NICKNAMES) {
			SetFocus(GetDlgItem(hwndDlg, IDC_NICK));
		} else {
			SendDlgItemMessage(hwndDlg, IDC_NICK, EM_SETREADONLY, (WPARAM)TRUE, 0);
		}
		if (dialog->getRoomName()!=NULL) {
			SetDlgItemTextA(hwndDlg, IDC_ROOM, dialog->getRoomName());
		}
		if (dialog->getNick()!=NULL) {
			SetDlgItemTextA(hwndDlg, IDC_NICK, dialog->getNick());
		}
		return FALSE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			MUCCEVENT muce;
			if (dialog->getRoomId()==NULL) {
				GetDlgItemTextA(hwndDlg, IDC_ROOM, str, 255);
				if (strlen(str)>0) {
					//dialog->setRoomId(str);
					dialog->setRoomName(str);
				}
			}
			GetDlgItemTextA(hwndDlg, IDC_NICK, str, 255);
			if (strlen(str)>0) {
				dialog->setNick(str);
			} else {
				dialog->setNick(NULL);
			}
			muce.iType = MUCC_EVENT_JOIN;
			muce.pszModule = dialog->getModule();
			muce.pszID = dialog->getRoomId();
			muce.pszName = dialog->getRoomName();
			muce.pszNick = dialog->getNick();
			muce.dwFlags = dialog->getRoomFlags();
			NotifyEventHooks(hHookEvent, 0,(WPARAM)&muce);
		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			return TRUE;
		}
		break;
	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;
	case WM_DESTROY:
		delete dialog;
		break;
	}
	return FALSE;
}

static void __cdecl ErrorDlgThread(void *vDialog)
{
	HelperDialog *dialog=(HelperDialog *)vDialog;
	MessageBoxA(NULL, dialog->getReason(), Translate("Error"), MB_OK | MB_ICONERROR);
	/*
	int result = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_GROUPCHAT_JOIN), NULL, JoinDlgProc, (LPARAM) dialog);
	if (result!=0) {
		MUCCEVENT muce;
		muce.iType = MUCC_EVENT_JOIN;
		muce.pszModule = dialog->getModule();
		muce.pszID = dialog->getRoomId();
		muce.pszName = dialog->getRoomName();
		muce.pszNick = dialog->getNick();
		NotifyEventHooks(hHookEvent, 0,(WPARAM)&muce);
	}
	*/
	delete dialog;
}

/*
static INT_PTR CALLBACK ErrorDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{	
	char str[256];
	HelperDialog *dialog;
	dialog = (HelperDialog *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch (msg) {
	case WM_INITDIALOG:
		dialog = (HelperDialog *) lParam;
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) dialog);
		dialog->setHWND(hwndDlg);
		SetFocus(GetDlgItem(hwndDlg, IDOK));
		if (dialog->getRoomName()!=NULL) {
			SetDlgItemTextA(hwndDlg, IDC_ROOM, dialog->getRoomName());
		}
		if (dialog->getNick()!=NULL) {
			SetDlgItemTextA(hwndDlg, IDC_NICK, dialog->getNick());
		}
		return FALSE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			if (dialog->getRoomId()==NULL) {
				GetDlgItemTextA(hwndDlg, IDC_ROOM, str, 255);
				if (strlen(str)>0) {
					//dialog->setRoomId(str);
					dialog->setRoomName(str);
				}
			}
			GetDlgItemTextA(hwndDlg, IDC_NICK, str, 255);
			if (strlen(str)>0) {
				dialog->setNick(str);
			} else {
				dialog->setNick(NULL);
			}
		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			return TRUE;
		}
		break;
	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;
	case WM_DESTROY:
		delete dialog;
		break;
	}
	return FALSE;
}
*/

static INT_PTR CALLBACK TopicDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{	
	char str[256];
	HelperDialog *dialog;
	dialog = (HelperDialog *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch (msg) {
	case WM_INITDIALOG:
		dialog = (HelperDialog *) lParam;
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) dialog);
		dialog->setHWND(hwndDlg);
		return FALSE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			MUCCEVENT muce;
			GetDlgItemTextA(hwndDlg, IDC_TOPIC, str, 255);
			dialog->setReason(str);
			muce.cbSize = sizeof(MUCCEVENT);
			muce.iType = MUCC_EVENT_TOPIC;
			muce.pszID = dialog->getRoomId();
			muce.pszModule = dialog->getModule();
			muce.pszText = dialog->getReason();
			NotifyEventHooks(hHookEvent, 0,(WPARAM)&muce);
		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			return TRUE;
		}
		break;
	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;
	case WM_DESTROY:
		delete dialog;
		break;
	}
	return FALSE;
}

