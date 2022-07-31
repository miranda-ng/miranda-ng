#if !defined(HISTORYSTATS_GUARD_STATISTIC_H)
#define HISTORYSTATS_GUARD_STATISTIC_H

#include "stdafx.h"
#include "_consts.h"

#include <vector>
#include <map>
#include <set>
#include <list>

#include "settings.h"
#include "message.h"

class CContact; // forward declaration instead of #include "contact.h"

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

	typedef std::vector<CContact*> ContactList;
	typedef std::vector<const CContact*> ContactListC;
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
	CContact* m_pTotals;
	CContact* m_pOmitted;

	bool
		m_bActuallyOmitted,  // did we really omit something
		m_bResult;           // result of calculations

	// start time for statistics
	uint32_t m_TimeStarted;
	uint32_t m_MSecStarted;

	// minimum/maximum date/time to include
	uint32_t m_TimeMin;
	uint32_t m_TimeMax;

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
	uint32_t m_nFirstTime;
	uint32_t m_nLastTime;

	// misc data
	uint32_t m_AverageMinTime;

private:
	// contact handling
	void prepareColumns();
	void prepareContactData(CContact& contact);
	void freeContactData(CContact& contact);
	void mergeContactData(CContact& contact, const CContact& include);
	void transformContactData(CContact& contact);
	CContact& addContact(const ext::string& nick, const ext::string& protoDisplayName, const ext::string& groupName, int nSources);

	// misc routines
	uint32_t getTimeStarted() { return m_TimeStarted; }
	bool shouldTerminate() { return (WaitForSingleObject(m_hCancelEvent, 0) == WAIT_OBJECT_0) || bool_(Miranda_IsTerminated()); }
	void handleAddMessage(CContact& contact, Message& msg);
	void handleAddChat(CContact& contact, bool bOutgoing, uint32_t localTimestampStarted, uint32_t duration);

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
	void createStatisticsSteps();
	static void __cdecl threadProc(Statistic *pStats);
	static void __cdecl threadProcSteps(Statistic *pStats);

public:
	~Statistic();

public:
	static void run(const Settings& settings, InvocationSource invokedFrom, HINSTANCE hInst, HWND hWndParent = nullptr);

	int countContacts() const { return m_Contacts.size(); }
	const CContact& getContact(int index) const;
	const CContact& getTotals() const { assert(m_pTotals); return *m_pTotals; }
	const CContact& getOmitted() const { assert(m_pOmitted); return *m_pOmitted; }
	bool hasTotals() const { return (m_pTotals != nullptr) && m_Settings.m_CalcTotals; } // MEMO: only makes sense after 'calc totals'-step
	bool hasOmitted() const { return (m_pOmitted != nullptr) && m_Settings.m_OmitContacts && m_Settings.m_OmittedInExtraRow && m_bActuallyOmitted; } // MEMO: only makes sense after 'omit'-step
	uint32_t getFirstTime(); // MEMO: only makes sense after 'calc totals'-step
	uint32_t getLastTime(); // MEMO: only makes sense after 'calc totals'-step
	uint32_t getHistoryTime(); // MEMO: only makes sense after 'calc totals'-step
	uint32_t getAverageMinTime() { return m_AverageMinTime; }

	// file management
	ext::string createFile(const ext::string& desiredName);
	bool newFile(const wchar_t* fileExt, ext::string& writeFile, ext::string& finalURL);
	bool newFilePNG(ext::string& writeFile, ext::string& finalURL) { return newFile(L".png", writeFile, finalURL); }
	bool newFilePNG(Canvas& canvas, ext::string& finalURL);
};

#endif // HISTORYSTATS_GUARD_STATISTIC_H