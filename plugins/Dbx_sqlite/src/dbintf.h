#pragma once

#define OWN_CACHED_CONTACT

#include <m_db_int.h>

struct DBCachedContact : public DBCachedContactBase
{
	uint32_t m_count;
	MEVENT   m_unread;
	uint32_t m_unreadTimestamp;

	DBCachedContact()
		: m_count(-1), m_unread(0) { }

	__forceinline bool HasCount() const {
		return m_count > -1;
	}

	void AddEvent(MEVENT hDbEvent, uint32_t timestamp, bool unread);
	void EditEvent(MEVENT hDbEvent, uint32_t timestamp, bool unread);
	void DeleteEvent(MEVENT hDbEvent);

	void MarkRead(MEVENT hDbEvent);
};

struct CDbxSQLiteEventCursor : public DB::EventCursor
{
	CDbxSQLiteEventCursor(MCONTACT _1, sqlite3* m_db, MEVENT hDbEvent, bool reverse = false);
	~CDbxSQLiteEventCursor() override;
	MEVENT FetchNext() override;
private:
	sqlite3* m_db;
	sqlite3_stmt* cursor;
};

class CDbxSQLite : public MDatabaseCommon, public MZeroedObject
{
	sqlite3 *m_db;

	sqlite3_stmt *evt_cur_fwd = nullptr, *evt_cur_backwd = nullptr;
	MCONTACT evt_cnt_fwd = 0, evt_cnt_backwd = 0;

	DBCachedContact m_system;

	struct Impl {
		CDbxSQLite &pro;

		CTimer m_timer;
		void OnTimer(CTimer *pTimer)
		{
			pTimer->Stop();
			pro.DBFlush(true);
		}

		Impl(CDbxSQLite &_p) :
			pro(_p),
			m_timer(Miranda_GetSystemWindow(), UINT_PTR(this))
		{
			m_timer.OnEvent = Callback(this, &Impl::OnTimer);
		}
	} m_impl;

	bool m_safetyMode;

	CDbxSQLite(sqlite3 *database);

	void InitContacts();
	void UninitContacts();

	LIST<char> m_modules;
	void InitEvents();
	void UninitEvents();

	void InitSettings();
	void UninitSettings();

	void DBFlush(bool bForce = false);

public:
	~CDbxSQLite();

	static int Create(const wchar_t *profile);
	static int Check(const wchar_t *profile);
	static MDatabaseCommon* Load(const wchar_t *profile, int readonly);

	STDMETHODIMP_(BOOL)     IsRelational(void) override;
	STDMETHODIMP_(void)     SetCacheSafetyMode(BOOL) override;

	STDMETHODIMP_(LONG)     GetContactCount(void) override;
	STDMETHODIMP_(LONG)     DeleteContact(MCONTACT contactID) override;
	STDMETHODIMP_(MCONTACT) AddContact(void) override;
	STDMETHODIMP_(BOOL)     IsDbContact(MCONTACT contactID) override;
	STDMETHODIMP_(LONG)     GetContactSize(void) override;

	STDMETHODIMP_(LONG)     GetEventCount(MCONTACT contactID) override;
	STDMETHODIMP_(MEVENT)   AddEvent(MCONTACT contactID, const DBEVENTINFO *dbe) override;
	STDMETHODIMP_(BOOL)     DeleteEvent(MEVENT hDbEvent) override;
	STDMETHODIMP_(BOOL)     EditEvent(MCONTACT contactID, MEVENT hDbEvent, const DBEVENTINFO *dbe) override;
	STDMETHODIMP_(LONG)     GetBlobSize(MEVENT hDbEvent) override;
	STDMETHODIMP_(BOOL)     GetEvent(MEVENT hDbEvent, DBEVENTINFO *dbe) override;
	STDMETHODIMP_(BOOL)     MarkEventRead(MCONTACT contactID, MEVENT hDbEvent) override;
	STDMETHODIMP_(MCONTACT) GetEventContact(MEVENT hDbEvent) override;
	STDMETHODIMP_(MEVENT)   FindFirstEvent(MCONTACT contactID) override;
	STDMETHODIMP_(MEVENT)   FindFirstUnreadEvent(MCONTACT contactID) override;
	STDMETHODIMP_(MEVENT)   FindLastEvent(MCONTACT contactID) override;
	STDMETHODIMP_(MEVENT)   FindNextEvent(MCONTACT contactID, MEVENT hDbEvent) override;
	STDMETHODIMP_(MEVENT)   FindPrevEvent(MCONTACT contactID, MEVENT hDbEvent) override;

	STDMETHODIMP_(MEVENT)   GetEventById(LPCSTR szModule, LPCSTR szId) override;

	STDMETHODIMP_(BOOL)     EnumModuleNames(DBMODULEENUMPROC pFunc, void *pParam) override;

	STDMETHODIMP_(BOOL)     ReadCryptoKey(MBinBuffer&) override;
	STDMETHODIMP_(BOOL)     StoreCryptoKey() override;

	STDMETHODIMP_(CRYPTO_PROVIDER*) ReadProvider() override;
	STDMETHODIMP_(BOOL)     StoreProvider(CRYPTO_PROVIDER*) override;

	STDMETHODIMP_(BOOL)     EnableEncryption(BOOL) override;
	STDMETHODIMP_(BOOL)     ReadEncryption() override;

	STDMETHODIMP_(BOOL)     WriteContactSettingWorker(MCONTACT contactID, DBCONTACTWRITESETTING &dbcws) override;
	STDMETHODIMP_(BOOL)     DeleteContactSetting(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting) override;
	STDMETHODIMP_(BOOL)     EnumContactSettings(MCONTACT hContact, DBSETTINGENUMPROC pfnEnumProc, const char *szModule, void *param) override;

	STDMETHODIMP_(BOOL)     MetaMergeHistory(DBCachedContact *ccMeta, DBCachedContact *ccSub) override;
	STDMETHODIMP_(BOOL)     MetaSplitHistory(DBCachedContact *ccMeta, DBCachedContact *ccSub) override;

	STDMETHODIMP_(BOOL)     Compact() override;
	STDMETHODIMP_(BOOL)     Backup(LPCWSTR) override;
	STDMETHODIMP_(BOOL)     Flush() override;

	STDMETHODIMP_(DATABASELINK*) GetDriver() override;

	STDMETHODIMP_(DB::EventCursor*) EventCursor(MCONTACT hContact, MEVENT hDbEvent) override;
	STDMETHODIMP_(DB::EventCursor*) EventCursorRev(MCONTACT hContact, MEVENT hDbEvent) override;
};
