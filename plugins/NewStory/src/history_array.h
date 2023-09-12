#ifndef __history_array__
#define __history_array__

enum
{
	GROUPING_NONE = 0,
	GROUPING_HEAD = 1,
	GROUPING_ITEM = 2,
};

struct ItemData
{
	MCONTACT hContact;
	MEVENT hEvent;

	bool m_bRtf;
	bool m_bSelected, m_bHighlighted;
	bool m_bLoaded, m_bIsResult;
	bool m_bOfflineFile, m_bOfflineDownloaded;
	uint8_t m_grouping;
	
	int savedTop, savedHeight;

	DB::EventInfo dbe;
	wchar_t *wtext;
	wchar_t *wszNick;

	HANDLE data;

	ItemData();
	~ItemData();

	ItemData* checkPrev(ItemData *pPrev);
	ItemData* checkPrevGC(ItemData *pPrev);
	void checkCreate(HWND hwnd);
	void setText();

	void load(bool bFullLoad);
	bool isLink(POINT pt, CMStringW *url = nullptr) const;
	bool isLinkChar(int idx) const;

	int getTemplate() const;
	int getCopyTemplate() const;
	void getFontColor(int &fontId, int &colorId) const;

	inline wchar_t *getWBuf()
	{
		load(true);
		return wtext;
	}
};

class Filter
{
	uint16_t flags;
	ptrW text;

public:
	enum
	{
		INCOMING = 0x001,
		OUTGOING = 0x002,
		MESSAGES = 0x004,
		FILES = 0x008,
		STATUS = 0x020,
		OTHER = 0x040,
		EVENTTEXT = 0x080,
		EVENTONLY = 0x100,
	};

	__forceinline Filter(uint16_t aFlags, const wchar_t *wText) :
		flags(aFlags),
		text(mir_wstrdup(wText))
	{
	}

	bool check(ItemData *item) const;
};

enum
{
	FILTER_TIME = 0x01,
	FILTER_TYPE = 0x02,
	FILTER_DIRECTION = 0x04,
	FILTER_TEXT = 0x08,
	FILTER_UNICODE = 0x10,

	FTYPE_MESSAGE = 0x01,
	FTYPE_FILE = 0x02,
	FTYPE_URL = 0x04,
	FTYPE_STATUS = 0x08,
	FTYPE_OTHER = 0x10,
	FTYPE_INCOMING = 0x20,
	FTYPE_OUTGOING = 0x40
};

#define HIST_BLOCK_SIZE 1000

struct ItemBlock : public MZeroedObject
{
	ItemData data[HIST_BLOCK_SIZE];
};

struct SearchResult
{
	SearchResult(MCONTACT _1, MEVENT _2, uint32_t _3) :
		hContact(_1),
		hEvent(_2),
		ts(_3)
	{}

	MCONTACT hContact;
	MEVENT hEvent;
	uint32_t ts;
};

class HistoryArray
{
	LIST<wchar_t> strings;
	OBJLIST<ItemBlock> pages;
	int iLastPageCounter = 0;

	ItemData& allocateItem(void);

public:
	HistoryArray();
	~HistoryArray();

	bool addEvent(MCONTACT hContact, MEVENT hEvent, int count);
	void addChatEvent(SESSION_INFO *si, const LOGINFO *pEvent);
	void addResults(const OBJLIST<SearchResult> &pArray);
	void clear();
	int  find(MEVENT hEvent);
	int  find(int id, int dir, const Filter &filter);
	int  getCount() const;
	void remove(int idx);
	void reset()
	{
		clear();
		pages.insert(new ItemBlock());
	}

	ItemData* get(int id, bool bLoad = false);
	
	__forceinline int FindNext(int id, const Filter &filter) { return find(id, +1, filter); }
	__forceinline int FindPrev(int id, const Filter &filter) { return find(id, -1, filter); }
};

#endif // __history_array__
