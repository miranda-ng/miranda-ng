#if !defined(HISTORYSTATS_GUARD_STATISTIC_H)
#define HISTORYSTATS_GUARD_STATISTIC_H

#include "stdafx.h"
#include "_consts.h"

#include <vector>
#include <map>
#include <set>
#include <list>

#include "settings.h"
#include "protocol.h"
#include "message.h"

class Contact; // forward declaration instead of #include "contact.h"

class Statistic
	: private pattern::NotCopyable<Statistic>
{
public:
	enum InvocationSource {
		fromOptions,
		fromStartup,
		fromMenu,
	};

	enum DBEventTypeRaw {
		EVENTTYPE_AVATARCHANGE        =  9003, // from pescuma
		EVENTTYPE_WAT_REQUEST         =  9601, // from WATrack
		EVENTTYPE_WAT_ANSWER          =  9602, // from WATrack
		EVENTTYPE_WAT_ERROR           =  9603, // from WATrack
		EVENTTYPE_STATUSCHANGE        = 25368, // from NewXStatusNotify
	};

	enum DBEventType {
		// messages
		etMessage             = EVENTTYPE_MESSAGE,
		// other events
		etFile                = EVENTTYPE_FILE,
		etURL                 = EVENTTYPE_URL,
		etICQSMS              = ICQEVENTTYPE_SMS,
		etICQWebPager         = ICQEVENTTYPE_WEBPAGER,
		etICQEMailExpress     = ICQEVENTTYPE_EMAILEXPRESS,
		// authorization and contacts
		etAuthRequest         = EVENTTYPE_AUTHREQUEST,
		etAdded               = EVENTTYPE_ADDED,
		etContacts            = EVENTTYPE_CONTACTS,
		// status/avatar/nick/... changes
		etStatusChange        = EVENTTYPE_STATUSCHANGE,
		etAvatarChange        = EVENTTYPE_AVATARCHANGE,
		// WaTrack events
		etWATRequest          = EVENTTYPE_WAT_REQUEST,
		etWATAnswer           = EVENTTYPE_WAT_ANSWER,
		etWATError            = EVENTTYPE_WAT_ERROR,
	};

	typedef std::vector<Contact*> ContactList;
	typedef std::vector<const Contact*> ContactListC;
	typedef std::pair<ext::string, ext::string> ConflictingFile; // (desired, temp)
	typedef std::list<ConflictingFile> ConflictingFiles;
	typedef std::map<Canvas::Digest, ext::string> ImageMap;

private:
	static bool m_bRunning;

private:
	// settings and the like
	Settings m_Settings;
	Settings::CharMapper m_CharMapper;

	// handles for windows and synchronisation
	HINSTANCE m_hInst;
	HWND m_hWndProgress;
	HANDLE m_hThreadPushEvent;
	HANDLE m_hCancelEvent;
	InvocationSource m_InvokedFrom;

	// list of contacts
	ContactList m_Contacts;

	// special 'contacts': omitted, totals
	Contact* m_pTotals;
	Contact* m_pOmitted;

	// did we really omit something
	bool m_bActuallyOmitted;

	// start time for statistics
	DWORD m_TimeStarted;
	DWORD m_MSecStarted;

	// minimum/maximum date/time to include
	DWORD m_TimeMin;
	DWORD m_TimeMax;

	// error processing and the like
	ext::string m_ErrorText;

	// column management
	int m_nNextSlot;
	std::vector<Column*> m_ActiveCols;
	std::vector<Column*> m_AcquireCols;
	std::vector<Column*> m_TransformCols;

	// file management
	ext::string m_TempPath;
	ext::string m_OutputPath;
	ext::string m_OutputFile;
	ext::string m_OutputPrefix;
	ConflictingFiles m_ConflictingFiles;
	int m_nLastFileNr;

	// PNG management
	ImageMap m_Images;

	// first/last message
	bool m_bHistoryTimeAvailable;
	DWORD m_nFirstTime;
	DWORD m_nLastTime;

	// misc data
	DWORD m_AverageMinTime;

private:
	// contact handling
	void prepareColumns();
	void prepareContactData(Contact& contact);
	void freeContactData(Contact& contact);
	void mergeContactData(Contact& contact, const Contact& include);
	void transformContactData(Contact& contact);
	Contact& addContact(const ext::string& nick, const ext::string& protoDisplayName, const ext::string& groupName, int nSources);

	// misc routines
	DWORD getTimeStarted() { return m_TimeStarted; }
	bool shouldTerminate() { return (WaitForSingleObject(m_hCancelEvent, 0) == WAIT_OBJECT_0) || (bool_(mu::system::terminated())); }
	void handleAddMessage(Contact& contact, Message& msg);
	void handleAddChat(Contact& contact, bool bOutgoing, DWORD localTimestampStarted, DWORD duration);

	// progress dialog handling
	static INT_PTR CALLBACK staticProgressProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	void setProgressMax(bool bSub, int max);
	void setProgressLabel(bool bSub, const ext::string& label);
	void stepProgress(bool bSub, int step = 1);
	static INT_PTR CALLBACK staticConflictProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// statistics creation steps
	bool stepInit();
	bool stepReadDB();
	bool stepRemoveContacts();
	bool stepSortContacts();
	bool stepPreOmitContacts();
	bool stepOmitContacts();
	bool stepCalcTotals();
	bool stepPostOmitContacts();
	bool stepTransformData();
	bool stepWriteHTML();

	// private constructor & main statistic creation routine
	explicit Statistic(const Settings& settings, InvocationSource invokedFrom, HINSTANCE hInst);
	bool createStatistics();
	bool createStatisticsSteps();
	static void __cdecl threadProc(void *lpParameter);
	static void __cdecl threadProcSteps(void *lpParameter);

public:
	~Statistic();

public:
	static void run(const Settings& settings, InvocationSource invokedFrom, HINSTANCE hInst, HWND hWndParent = NULL);

	int countContacts() const { return m_Contacts.size(); }
	const Contact& getContact(int index) const;
	const Contact& getTotals() const { assert(m_pTotals); return *m_pTotals; }
	const Contact& getOmitted() const { assert(m_pOmitted); return *m_pOmitted; }
	bool hasTotals() const { return (m_pTotals != NULL) && m_Settings.m_CalcTotals; } // MEMO: only makes sense after 'calc totals'-step
	bool hasOmitted() const { return (m_pOmitted != NULL) && m_Settings.m_OmitContacts && m_Settings.m_OmittedInExtraRow && m_bActuallyOmitted; } // MEMO: only makes sense after 'omit'-step
	DWORD getFirstTime(); // MEMO: only makes sense after 'calc totals'-step
	DWORD getLastTime(); // MEMO: only makes sense after 'calc totals'-step
	DWORD getHistoryTime(); // MEMO: only makes sense after 'calc totals'-step
	DWORD getAverageMinTime() { return m_AverageMinTime; }

	// file management
	ext::string createFile(const ext::string& desiredName);
	bool newFile(const TCHAR* fileExt, ext::string& writeFile, ext::string& finalURL);
	bool newFilePNG(ext::string& writeFile, ext::string& finalURL) { return newFile(_T(".png"), writeFile, finalURL); }
	bool newFilePNG(Canvas& canvas, ext::string& finalURL);
};

#endif // HISTORYSTATS_GUARD_STATISTIC_H