#ifndef __m_notify_h__
#define __m_notify_h__

#include <m_database.h>

/*** Miranda Notify Dispatcher ************************************************\
Notify Dispatcher provides common interface to different notification plugins
like osd, popup, ticker etc.
\******************************************************************************/

/* Options UI event and service. The same as for miranda options */
#define ME_NOTIFY_OPT_INITIALISE "Notify/Opt/Initialise"
#define MS_NOTIFY_OPT_ADDPAGE "Notify/Opt/AddPage"

#define UM_MNOTIFY_CHECK (WM_USER+100)

typedef struct tagMNOTIFYACTIONINFO {
	HICON icon;
	char name[MAXMODULELABELLENGTH];
	char service[MAXMODULELABELLENGTH];
	DWORD cookie;
} MNOTIFYACTIONINFO;

// Just like miranda pluginLink... This should work faster then services,
// we need some reactivity in notifications.
typedef struct tagMNNOTIFYLINK
{
	/* Create a new notification type */
	HANDLE (*Register)(const char *name, HICON icon);

	// Create a new notification object
	HANDLE (*Create)(HANDLE type);

	// Check is handle is a valid notification object
	int (*IsValid)(HANDLE notify);

	// Set/get information about object, or type defaults
	int (*Set)(HANDLE notifyORtype, const char *name, DBVARIANT val);
	int (*Get)(HANDLE notifyORtype, const char *name, DBVARIANT *val);

	// Set/get actions
	int (*AddAction)(HANDLE notifyORtype, HICON icon, const char *name, const char *service, DWORD cookie);
	int (*GetActions)(HANDLE notifyORtype, MNOTIFYACTIONINFO *actions); // pass NULL to get total number of actions

	// Increment/decrement refer count of notification object. Unreferred objects are destroyed
	int (*AddRef)(HANDLE notify);
	int (*Release)(HANDLE notify);

	// Notify user
	void (*Show)(HANDLE notify);
	void (*Update)(HANDLE notify);
	void (*Remove)(HANDLE notify);
} MNOTIFYLINK;

// Get the MNOTIFYLINK struct
// result = (LRESULT)(MNOTIFYLINK*)notifyLink
#define MS_NOTIFY_GETLINK "Notify/GetLink"

// Hook this to process corresponding actions
#define ME_NOTIFY_SHOW "Notify/Show"
#define ME_NOTIFY_UPDATE "Notify/Update"
#define ME_NOTIFY_REMOVE "Notify/Remove"

#if !defined(MNOTIFY_NOEXTERN)
	extern
	#ifdef __cpluplus
		"C"
	#endif
	MNOTIFYLINK *notifyLink;
#endif

#if !defined(MNOTIFY_NOHELPERS) && !defined(MNOTIFY_NOEXTERN)
	#define MNotifyRegister(a,b)     (notifyLink?notifyLink->Register((a),(b)):0)
	#define MNotifyCreate(a)         (notifyLink?notifyLink->Create((a)):0)
	#define MNotifyIsValid(a)        (notifyLink?notifyLink->IsValid((a)):0)
	#define MNotifySet(a,b,c)        (notifyLink?notifyLink->Set((a),(b),(c)):0)
	#define MNotifyGet(a,b,c)        (notifyLink?notifyLink->Get((a),(b),(c)):0)
	#define MNotifyAddAction(a,b,c)  (notifyLink?notifyLink->AddAction((a),(b),(c)):0)
	#define MNotifyGetActions(a,b)   (notifyLink?notifyLink->GetActions((a),(b)):0)
	#define MNotifyGet(a,b,c)        (notifyLink?notifyLink->Get((a),(b),(c)):0)
	#define MNotifyAddRef(a)         (notifyLink?notifyLink->AddRef((a)):0)
	#define MNotifyRelease(a)        (notifyLink?notifyLink->Release((a)):0)
	#define MNotifyShow(a)           (notifyLink?notifyLink->Show(a):0)
	#define MNotifyUpdate(a)         (notifyLink?notifyLink->Update(a):0)
	#define MNotifyRemove(a)         (notifyLink?notifyLink->Remove(a):0)

	static void __inline MNotifyGetLink()
	{
		notifyLink = ServiceExists(MS_NOTIFY_GETLINK) ? (MNOTIFYLINK *)CallService(MS_NOTIFY_GETLINK,0,0) : 0;
	}

	// get helpers
	static __inline BYTE MNotifyGetByte(HANDLE notifyORtype, const char *name, BYTE defValue)
	{
		DBVARIANT dbv;
		MNotifyGet(notifyORtype, name, &dbv);
		if (dbv.type != DBVT_BYTE) return defValue;
		return dbv.bVal;
	}
	static __inline WORD MNotifyGetWord(HANDLE notifyORtype, const char *name, WORD defValue)
	{
		DBVARIANT dbv;
		MNotifyGet(notifyORtype, name, &dbv);
		if (dbv.type != DBVT_WORD) return defValue;
		return dbv.wVal;
	}
	static __inline DWORD MNotifyGetDWord(HANDLE notifyORtype, const char *name, DWORD defValue)
	{
		DBVARIANT dbv;
		MNotifyGet(notifyORtype, name, &dbv);
		if (dbv.type != DBVT_DWORD) return defValue;
		return dbv.dVal;
	}
	static __inline const char *MNotifyGetString(HANDLE notifyORtype, const char *name, const char *defValue)
	{
		DBVARIANT dbv;
		MNotifyGet(notifyORtype, name, &dbv);
		if (dbv.type != DBVT_ASCIIZ) return defValue;
		return dbv.pszVal;
	}
	static __inline const WCHAR *MNotifyGetWString(HANDLE notifyORtype, const char *name, const WCHAR *defValue)
	{
		DBVARIANT dbv;
		MNotifyGet(notifyORtype, name, &dbv);
		if (dbv.type != DBVT_WCHAR) return defValue;
		return dbv.pwszVal;
	}

	// set helpers
	static __inline void MNotifySetByte(HANDLE notifyORtype, const char *name, BYTE value)
	{
		DBVARIANT dbv;
		dbv.type = DBVT_BYTE;
		dbv.bVal = value;
		MNotifySet(notifyORtype, name, dbv);
	}
	static __inline void MNotifySetWord(HANDLE notifyORtype, const char *name, WORD value)
	{
		DBVARIANT dbv;
		dbv.type = DBVT_WORD;
		dbv.wVal = value;
		MNotifySet(notifyORtype, name, dbv);
	}
	static __inline void MNotifySetDWord(HANDLE notifyORtype, const char *name, DWORD value)
	{
		DBVARIANT dbv;
		dbv.type = DBVT_DWORD;
		dbv.dVal = value;
		MNotifySet(notifyORtype, name, dbv);
	}
	static __inline void MNotifySetString(HANDLE notifyORtype, const char *name, const char *value)
	{
		DBVARIANT dbv;
		dbv.type = DBVT_ASCIIZ;
		dbv.pszVal = (char *)value;
		MNotifySet(notifyORtype, name, dbv);
	}
	static __inline void MNotifySetWString(HANDLE notifyORtype, const char *name, const WCHAR *value)
	{
		DBVARIANT dbv;
		dbv.type = DBVT_WCHAR;
		dbv.pwszVal = (WCHAR *)value;
		MNotifySet(notifyORtype, name, dbv);
	}
#endif

// Common options for Get/Set actions
#define NFOPT_TYPENAME  "General/TypeName"
#define NFOPT_ICON      "General/Icon"
#define NFOPT_CONTACT   "General/Contact"
#define NFOPT_EVENT     "General/Event"
#define NFOPT_TEXT      "General/Text"
#define NFOPT_TEXTW     "General/TextW"
#define NFOPT_TITLE     "General/Title"
#define NFOPT_TITLEW    "General/TitleW"
#define NFOPT_BACKCOLOR "General/BackColor"
#define NFOPT_TEXTCOLOR "General/TextColor"
#define NFOPT_TIMEOUT   "General/Timeout"

#endif // __m_notify_h__
