#ifndef __history_array__
#define __history_array__

struct ItemData
{
	enum EventLoadMode
	{
		ELM_NOTHING,
		ELM_INFO,
		ELM_DATA
	};

	BYTE flags = 0;

	MCONTACT hContact = 0;
	MEVENT hEvent = 0;

	bool dbeOk = false;
	DBEVENTINFO dbe;

	bool wtext_del = false;
	wchar_t *wtext = 0;

	wchar_t *wszNick = 0;

	HANDLE data = 0;

	ItemData() { memset(&dbe, 0, sizeof(dbe)); }
	~ItemData();

	bool load(EventLoadMode mode);
	inline bool loadInline(EventLoadMode mode)
	{
		if (((mode >= ItemData::ELM_INFO) && !dbeOk) || ((mode == ItemData::ELM_DATA) && !dbe.pBlob))
			return load(mode);
		return true;
	}
	inline wchar_t *getWBuf()
	{
		loadInline(ItemData::ELM_DATA);
		return wtext;
	}
};

class Filter
{
	WORD flags;
	int *refCount;
	wchar_t *text;

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

	Filter(WORD aFlags, wchar_t *wText)
	{
		refCount = new int(0);
		flags = aFlags;
		text = new wchar_t[mir_wstrlen(wText) + 1];
		mir_wstrcpy(text, wText);
	}
	Filter(const Filter &other)
	{
		flags = other.flags;
		refCount = other.refCount;
		text = other.text;
		++ *refCount;
	}
	Filter &operator=(const Filter &other)
	{
		flags = other.flags;
		refCount = other.refCount;
		text = other.text;
		++ *refCount;
	}
	~Filter()
	{
		if (!-- * refCount) {
			delete refCount;
			if (text) delete[] text;
		}
	}
	
	bool check(ItemData *item);
};

enum
{
	HIF_SELECTED = 0x01,

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

class HistoryArray
{
	LIST<wchar_t> strings;
	OBJLIST<ItemBlock> pages;
	int iLastPageCounter = 0;

	ItemData& allocateItem(void);

public:
	HistoryArray();
	~HistoryArray();

	bool addEvent(MCONTACT hContact, MEVENT hEvent, int count, ItemData::EventLoadMode mode = ItemData::ELM_NOTHING);
	void addChatEvent(SESSION_INFO *si, LOGINFO *pEvent);
	void clear();
	int  getCount() const;
	void reset()
	{
		clear();
		pages.insert(new ItemBlock());
	}

	//	bool preloadEvents(int count = 10);

	ItemData* get(int id, ItemData::EventLoadMode mode = ItemData::ELM_NOTHING);
	ItemData* operator[] (int id) { return get(id, ItemData::ELM_DATA); }
	ItemData* operator() (int id) { return get(id, ItemData::ELM_INFO); }

	int FindRel(int id, int dir, Filter filter)
	{
		int count = getCount();
		for (int i = id + dir; (i >= 0) && (i < count); i += dir)
			if (filter.check(get(i)))
				return i;
		return -1;
	}
	int FindNext(int id, Filter filter) { return FindRel(id, +1, filter); }
	int FindPrev(int id, Filter filter) { return FindRel(id, -1, filter); }
};

#endif // __history_array__
