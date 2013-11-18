#ifndef _USERLIST_INC
#define _USERLIST_INC

#include "common.h"
#include "utils.h"

#include <fstream>
#include <string>
#include <iostream>

#include "clist_util.h"

typedef struct {
	int cbSize;
	char *nick;
	char *firstName;
	char *lastName;
	char *email;
	char reserved[16];

	char name[256];
	char stid[256];
	bool group;

} MYPROTOSEARCHRESULT;

typedef struct {
	size_t nSize;
	int nFieldCount;
	TCHAR ** pszFields;
	MYPROTOSEARCHRESULT psr;
} MYCUSTOMSEARCHRESULTS;

HANDLE FindContactByUserId(const char *id);
bool GetAwareIdFromContact(HANDLE hContact, mwAwareIdBlock *id_block);

int SearchForUser(const char *name);
int GetMoreDetails(const char *name);

int CreateSearchDialog(WPARAM wParam, LPARAM lParam);
int SearchFromDialog(WPARAM wParam, LPARAM lParam);

HANDLE AddSearchedUser(MYPROTOSEARCHRESULT *mpsr, bool temporary);

HANDLE AddContact(mwSametimeUser *user, bool temporary);

void UserListCreate();
void UserListAddStored();
void UserListDestroy();

void ImportContactsFromFile(TCHAR *filename);
void ExportContactsToServer();

void UserRecvAwayMessage(HANDLE hContact);

void InitUserList(mwSession *session);
void DeinitUserList(mwSession *session);

#endif