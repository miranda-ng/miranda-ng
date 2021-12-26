#ifndef __history_array__
#define __history_array__

struct ItemData
{
	MCONTACT hContact;
	MEVENT hEvent;

	bool bSelected;
	bool bLoaded;
	int savedTop;

	DBEVENTINFO dbe;
	wchar_t *wtext;
	wchar_t *wszNick;

	HANDLE data;
	ItemData *pPrev;

	ItemData() { memset(this, 0, sizeof(*this)); }
	~ItemData();

	void checkCreate(HWND hwnd);

	void load(bool bFullLoad);
	bool isGrouped() const;
	bool isLink(POINT pt) const;

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

	Filter(uint16_t aFlags, wchar_t *wText)
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
			delete[] text;
		}
	}
	
	bool check(ItemData *item);
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
	void addChatEvent(SESSION_INFO *si, LOGINFO *pEvent);
	void clear();
	int  getCount() const;
	void reset()
	{
		clear();
		pages.insert(new ItemBlock());
	}

	//	bool preloadEvents(int count = 10);

	ItemData* get(int id, bool bLoad = false);
	ItemData* operator[] (int id) { return get(id, true); }

	void remove(int idx);

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
