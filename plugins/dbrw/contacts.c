/*
dbRW
Copyright (c) 2005-2009 Robert Rainwater

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
#include "dbrw.h"

static CRITICAL_SECTION csContactsDb;
static SortedList sContactList;

static int contacts_compare(void* p1, void* p2);

enum {
	SQL_CTC_STMT_DELETE=0,
	SQL_CTC_STMT_ADD,
	SQL_CTC_STMT_NUM
};
static char *ctc_stmts[SQL_CTC_STMT_NUM] = {
	"DELETE FROM dbrw_contacts WHERE id = ?;",
	"INSERT INTO dbrw_contacts VALUES(NULL,?);"
};
static sqlite3_stmt *ctc_stmts_prep[SQL_CTC_STMT_NUM] = {0};

void contacts_init() {
	InitializeCriticalSection(&csContactsDb);
	ZeroMemory(&sContactList, sizeof(sContactList));
	sContactList.increment = 50;
	sContactList.sortFunc = contacts_compare;
	sql_prepare_add(ctc_stmts, ctc_stmts_prep, SQL_CTC_STMT_NUM);
	{
		sqlite3_stmt *st = NULL;
		int id, idx;

		if (sql_prepare(g_sqlite, "SELECT * FROM dbrw_contacts;", &st)!=SQLITE_OK) {
			return;
		}
		while (sql_step(st)==SQLITE_ROW) {
			id = (int)sqlite3_column_int(st, 0);
			if(!li.List_GetIndex(&sContactList, (void*)id, &idx)) {
				li.List_Insert(&sContactList, (void*)id, idx);
			}
		}
		sql_finalize(st);
	}
}

void contacts_destroy() {
	li.List_Destroy(&sContactList);
	DeleteCriticalSection(&csContactsDb);
}

static int contacts_compare(void* p1, void* p2) {
	return (int)p1-(int)p2;
}

static int contacts_isRealContact(int id) {
	int idx;

	if(li.List_GetIndex(&sContactList, (void*)id, &idx))
		return 1;
	log1("Invalid contact id requested (%d)", id);
	return 0;
}

INT_PTR contacts_getCount(WPARAM wParam, LPARAM lParam) {
	int rc;

	EnterCriticalSection(&csContactsDb);
	rc = sContactList.realCount;
	LeaveCriticalSection(&csContactsDb);
	return rc;
}

INT_PTR contacts_findFirst(WPARAM wParam, LPARAM lParam) {
	int rc;

	EnterCriticalSection(&csContactsDb);
	if (sContactList.realCount==0) {
		LeaveCriticalSection(&csContactsDb);
		return 0;
	}
	rc = (int)sContactList.items[0];
	LeaveCriticalSection(&csContactsDb);
	return rc;
}

INT_PTR contacts_findNext(WPARAM wParam, LPARAM lParam) {
	int id = (int)wParam;
	int rc, idx;

	EnterCriticalSection(&csContactsDb);
	if (sContactList.realCount==0) {
		LeaveCriticalSection(&csContactsDb);
		return 0;
	}
	if(!li.List_GetIndex(&sContactList, (void*)id, &idx)) {
		LeaveCriticalSection(&csContactsDb);
		return 0;
	}
	if (idx>=(sContactList.realCount-1)) {
		LeaveCriticalSection(&csContactsDb);
		return 0;
	}
	rc = (int)sContactList.items[idx+1];
	LeaveCriticalSection(&csContactsDb);
	return rc;
}

INT_PTR contacts_delete(WPARAM wParam, LPARAM lParam) {
	int id = (int)wParam;
	int rc = 0;
	
	EnterCriticalSection(&csContactsDb);
	if (!contacts_isRealContact(id)) {
		rc = 1;
	}
	else {
		int idx;
		
        LeaveCriticalSection(&csContactsDb);
		NotifyEventHooks(hContactDeletedEvent, wParam, 0);
        EnterCriticalSection(&csContactsDb);
		li.List_GetIndex(&sContactList, (void*)id, &idx);
		li.List_Remove(&sContactList, idx);
		
        // Begin Transaction
        sql_stmt_begin();
        
		// Delete contact
		sqlite3_bind_int(ctc_stmts_prep[SQL_CTC_STMT_DELETE], 1, id);
		sql_step(ctc_stmts_prep[SQL_CTC_STMT_DELETE]);
		sql_reset(ctc_stmts_prep[SQL_CTC_STMT_DELETE]);
        
		// Delete contact's settings
		settings_deleteContactData((HANDLE)id);

		// Delete contact's events
        events_deleteContactData((HANDLE)id);
        
        // Commit transaction
        sql_stmt_end();
        log1("Deleted contact (%d) and associated data", id);
	}
	LeaveCriticalSection(&csContactsDb);
	return rc;
}

INT_PTR contacts_add(WPARAM wParam, LPARAM lParam) {
	int id = 0;
	int idx;

	EnterCriticalSection(&csContactsDb);
	sqlite3_bind_int(ctc_stmts_prep[SQL_CTC_STMT_ADD], 1, time(NULL));
	if (sql_step(ctc_stmts_prep[SQL_CTC_STMT_ADD])==SQLITE_DONE) {
		id = (int)sqlite3_last_insert_rowid(g_sqlite);
		if (!li.List_GetIndex(&sContactList, (void*)id, &idx))
			li.List_Insert(&sContactList, (void*)id, idx);
	}
	sql_reset(ctc_stmts_prep[SQL_CTC_STMT_ADD]);
	LeaveCriticalSection(&csContactsDb);
	if (id)
		NotifyEventHooks(hContactAddedEvent, (WPARAM)id, 0);
    else 
        log0("Error creating contact");
	return id;
}

INT_PTR contacts_isContact(WPARAM wParam, LPARAM lParam) {
	int id = (int)wParam;
	int rc = 0;

	EnterCriticalSection(&csContactsDb);
	if (contacts_isRealContact(id))
		rc = 1;
	LeaveCriticalSection(&csContactsDb);
	return rc;
}
