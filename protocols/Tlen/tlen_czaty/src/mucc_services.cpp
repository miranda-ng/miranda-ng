/*

MUCC Group Chat GUI Plugin for Miranda NG
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
#include "mucc_services.h"
#include "Utils.h"
#include "HelperDialog.h"

int MUCCQueryChatGroups(MUCCQUERYRESULT *queryResult) 
{
	ManagerWindow *managerWnd=ManagerWindow::getWindow(queryResult->pszModule);
	if (managerWnd != NULL) {
		managerWnd->queryResultGroups(queryResult);
	}
	return 1;
}

int MUCCQueryChatRooms(MUCCQUERYRESULT *queryResult) 
{
	ManagerWindow *managerWnd=ManagerWindow::getWindow(queryResult->pszModule);
	if (managerWnd != NULL) {
		managerWnd->queryResultRooms(queryResult);
	}
	return 1;
}

int MUCCQueryUserRooms(MUCCQUERYRESULT *queryResult) 
{
	ManagerWindow *managerWnd=ManagerWindow::getWindow(queryResult->pszModule);
	if (managerWnd != NULL) {
		managerWnd->queryResultUserRooms(queryResult);
	}
	return 1;
}

int MUCCQueryUserNicks(MUCCQUERYRESULT *queryResult) 
{
	ManagerWindow *managerWnd=ManagerWindow::getWindow(queryResult->pszModule);
	if (managerWnd != NULL) {
		managerWnd->queryResultUserNick(queryResult);
	}
	return 1;
}

INT_PTR MUCCQueryResult(WPARAM wParam, LPARAM lParam)
{
	MUCCQUERYRESULT *queryResult=(MUCCQUERYRESULT *)lParam;
	ChatWindow * chatWindow = NULL;
	switch (queryResult->iType) {
		case MUCC_EVENT_QUERY_GROUPS:
			MUCCQueryChatGroups(queryResult);
			break;
		case MUCC_EVENT_QUERY_ROOMS:
			MUCCQueryChatRooms(queryResult);
			break;
		case MUCC_EVENT_QUERY_USER_ROOMS:
			MUCCQueryUserRooms(queryResult);
			break;
		case MUCC_EVENT_QUERY_USER_NICKS:
			MUCCQueryUserNicks(queryResult);
			break;
		case MUCC_EVENT_QUERY_CONTACTS:
		case MUCC_EVENT_QUERY_USERS:
			chatWindow=ChatWindow::getWindow(queryResult->pszModule, queryResult->pszParent);
			if (chatWindow != NULL) {
				SendMessage(chatWindow->getHWND(), DM_CHAT_QUERY, (WPARAM) 0, (LPARAM) queryResult);
			}
			break;
	}
	return 0;
}

INT_PTR MUCCNewWindow(WPARAM wParam, LPARAM lParam)
{
	MUCCWINDOW *mucWindow = (MUCCWINDOW *) lParam;
	if (mucWindow->iType == MUCC_WINDOW_CHATROOM) {
		ChatWindow *chat = ChatWindow::getWindow(mucWindow->pszModule, mucWindow->pszID);
		if (chat == NULL) {
			chat = new ChatWindow(mucWindow);
		}
	} else if (mucWindow->iType == MUCC_WINDOW_CHATLIST) {
		ManagerWindow *manager = ManagerWindow::getWindow(mucWindow->pszModule);
		if (manager == NULL) {
			
		}
		//Utils::log("setting name: %s", mucWindow->pszModuleName);
		manager->setModuleName(mucWindow->pszModuleName);
		manager->start();
	}
	return 0;
}

INT_PTR MUCCEvent(WPARAM wParam, LPARAM lParam)
{
	MUCCEVENT* mucEvent = (MUCCEVENT *) lParam;
	ChatWindow * chatWindow = NULL;
	switch (mucEvent->iType) {
		case MUCC_EVENT_STATUS:
		case MUCC_EVENT_MESSAGE:
		case MUCC_EVENT_ROOM_INFO:
		case MUCC_EVENT_LEAVE:
		case MUCC_EVENT_TOPIC:
			chatWindow = ChatWindow::getWindow(mucEvent->pszModule, mucEvent->pszID);
			if (chatWindow != NULL) {
				SendMessage(chatWindow->getHWND(), DM_CHAT_EVENT, (WPARAM) 0, (LPARAM) mucEvent);
			}
			break;
		case MUCC_EVENT_ERROR:
			chatWindow = ChatWindow::getWindow(mucEvent->pszModule, mucEvent->pszID);
			if (chatWindow != NULL) {
				SendMessage(chatWindow->getHWND(), DM_CHAT_EVENT, (WPARAM) 0, (LPARAM) mucEvent);
			} else {
				HelperDialog::errorDlg(mucEvent);
			}
			break;
		case MUCC_EVENT_INVITATION:
			HelperDialog::acceptDlg(mucEvent);
			break;
		case MUCC_EVENT_JOIN:
			HelperDialog::joinDlg(mucEvent);
			break;
	}
	return 0;
}
