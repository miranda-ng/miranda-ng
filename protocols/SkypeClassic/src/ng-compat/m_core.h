#define IS_MIRANDAIM

typedef HANDLE MCONTACT;

#define db_get_ws DBGetContactSettingWString
#define db_free DBFreeVariant
#define db_get_b DBGetContactSettingByte
#define db_get_s DBGetContactSettingString
#define db_get_ts DBGetContactSettingTString
#define db_set_s DBWriteContactSettingString
#define db_set_b DBWriteContactSettingByte
#define db_get_w DBGetContactSettingWord
#define db_set_w DBWriteContactSettingWord
#define db_set_ts DBWriteContactSettingTString
#define db_get_dw DBGetContactSettingDword
#define db_set_dw DBWriteContactSettingDword
#define db_set_utf SkypeDBWriteContactSettingUTF8String
#define db_set_ws DBWriteContactSettingWString


#define Menu_AddContactMenuItem(mi) (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)mi)
#define Menu_AddMainMenuItem(mi)	(HANDLE)CallService(MS_CLIST_ADDMAINMENUITEM, (WPARAM)NULL,(LPARAM)mi)

#define db_find_first() (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0)
#define db_find_next(hContact) (HANDLE)CallService( MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0)
#define db_event_getContact(hContact) (HANDLE)CallService (MS_DB_EVENT_GETCONTACT, (WPARAM)hContact, 0)
#define db_event_delete(hContact,hEvent) CallService (MS_DB_EVENT_DELETE, (WPARAM)hContact, (LPARAM)hEvent)
#define db_event_first(hContact) (HANDLE)CallService(MS_DB_EVENT_FINDFIRST,(WPARAM)hContact,0)
#define db_event_get(hDbEvent,dbei) CallService(MS_DB_EVENT_GET,(WPARAM)hDbEvent,(LPARAM)dbei)
#define db_event_last(hContact) (HANDLE)CallService(MS_DB_EVENT_FINDLAST,(WPARAM)hContact,0)
#define db_event_add(hContact,dbei) (HANDLE)CallServiceSync(MS_DB_EVENT_ADD, (WPARAM)(HANDLE)hContact, (LPARAM)dbei)
#define db_event_firstUnread(hContact) (HANDLE)CallService(MS_DB_EVENT_FINDFIRSTUNREAD,(WPARAM)hContact,0)
#define db_event_markRead(hContact,hDbEvent) CallService(MS_DB_EVENT_MARKREAD,(WPARAM)hContact,(LPARAM)hDbEvent)
#define db_event_getBlobSize(hEvent) CallService(MS_DB_EVENT_GETBLOBSIZE, (WPARAM)hEvent, 0)
#define db_mc_isSub(hContact) DBGetContactSettingByte(hContact, "MetaContacts", "IsSubcontact", 0)
#define db_event_next(hContact,hDbEvent) (HANDLE)CallService(MS_DB_EVENT_FINDNEXT,(WPARAM)hDbEvent,0)

#define Thread_Push(a,b) CallService(MS_SYSTEM_THREAD_PUSH, a, b)
#define Thread_Pop() CallService(MS_SYSTEM_THREAD_POP, 0, 0);

#define GetContactProto(hContact) (char*)CallService( MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0 )

#define Options_AddPage(h,odp) CallService(MS_OPT_ADDPAGE, h, (LPARAM)odp);
#define UserInfo_AddPage(h,odp) CallService(MS_USERINFO_ADDPAGE, h, (LPARAM)odp);

#define _A2T(x) x
#define _T2A(x) x
#define SIZEOF(x) sizeof(x)/sizeof(x[0])
#define GSMDF_TCHAR GCMDF_TCHAR

// Write contact setting as UTF-8 for convenience, if possible. Older Miranda IM versions will store it as ANSI
INT_PTR SkypeDBWriteContactSettingUTF8String(HANDLE hContact,const char *szModule,const char *szSetting,const char *val);
