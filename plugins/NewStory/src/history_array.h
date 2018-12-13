#ifndef __history_array__
#define __history_array__

enum EventLoadMode
{
	ELM_NOTHING,
	ELM_INFO,
	ELM_DATA
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

class HistoryArray
{
public:
	struct ItemData
	{
		BYTE flags;

		MCONTACT hContact;
		MEVENT hEvent;

		bool dbeOk;
		DBEVENTINFO dbe;

		bool atext_del, wtext_del;
		char *atext;
		WCHAR *wtext;

		HANDLE data;

		ItemData() : flags(0), hContact(0), hEvent(0), atext(0), wtext(0), atext_del(false), wtext_del(false), data(0), dbeOk(false) {}
		~ItemData();
		bool load(EventLoadMode mode);
		inline bool loadInline(EventLoadMode mode)
		{
			if (((mode >= ELM_INFO) && !dbeOk) || ((mode == ELM_DATA) && !dbe.pBlob))
				return load(mode);
			return true;
		}
		inline TCHAR *getTBuf()
		{
			loadInline(ELM_DATA);
			#ifdef UNICODE
			return wtext;
			#else
			return atext;
			#endif
		}
		inline char *getBuf()
		{
			loadInline(ELM_DATA);
			return atext;
		}
		inline WCHAR *getWBuf()
		{
			loadInline(ELM_DATA);
			return wtext;
		}
	};

	struct ItemBlock
	{
		ItemData *items;
		int count;
		ItemBlock *prev, *next;
	};

	class Filter
	{
	public:
		enum
		{
			INCOMING = 0x001,
			OUTGOING = 0x002,
			MESSAGES = 0x004,
			FILES = 0x008,
			URLS = 0x010,
			STATUS = 0x020,
			OTHER = 0x040,
			EVENTTEXT = 0x080,
			EVENTONLY = 0x100,
		};
		Filter(WORD aFlags, TCHAR *aText)
		{
			refCount = new int(0);
			flags = aFlags;
			text = new TCHAR[lstrlen(aText) + 1];
			lstrcpy(text, aText);
		}
		Filter(const Filter &other)
		{
			flags = other.flags;
			refCount = other.refCount;
			text = other.text;
			++*refCount;
		}
		Filter &operator=(const Filter &other)
		{
			flags = other.flags;
			refCount = other.refCount;
			text = other.text;
			++*refCount;
		}
		~Filter()
		{
			if (!--*refCount) {
				delete refCount;
				if (text) delete[] text;
			}
		}
		inline bool check(ItemData *item)
		{
			if (!item) return false;
			if (!(flags & EVENTONLY)) {
				if (item->dbe.flags & DBEF_SENT) {
					if (!(flags & OUTGOING))
						return false;
				}
				else {
					if (!(flags & INCOMING))
						return false;
				}
				switch (item->dbe.eventType) {
				case EVENTTYPE_MESSAGE:
					if (!(flags & MESSAGES))
						return false;
					break;
				case EVENTTYPE_FILE:
					if (!(flags & FILES))
						return false;
					break;
				case EVENTTYPE_URL:
					if (!(flags & URLS))
						return false;
					break;
				case EVENTTYPE_STATUSCHANGE:
					if (!(flags & STATUS))
						return false;
					break;
				default:
					if (!(flags & OTHER))
						return false;
				}
			}
			if (flags & (EVENTTEXT | EVENTONLY)) {
				item->loadInline(ELM_DATA);
				return CheckFilter(item->getTBuf(), text);
			}
			return true;
		};

	private:
		WORD flags;
		int *refCount;
		TCHAR *text;
	};

private:
	ItemBlock *head, *tail;
	ItemBlock *preBlock;
	int preIndex;
	bool allocateBlock(int count);

	bool caching;
	bool caching_complete;
	CRITICAL_SECTION csItems;
	static void CacheThreadFunc(void *arg);

public:
	HistoryArray();
	~HistoryArray();

	void clear();
	bool addHistory(MCONTACT hContact, EventLoadMode mode = ELM_NOTHING);
	bool addEvent(MCONTACT hContact, MEVENT hEvent, EventLoadMode mode = ELM_NOTHING);

	//	bool preloadEvents(int count = 10);

	ItemData *get(int id, EventLoadMode mode = ELM_NOTHING);
	ItemData *operator[] (int id) { return get(id, ELM_DATA); }
	ItemData *operator() (int id) { return get(id, ELM_INFO); }

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

	int getCount()
	{
		int res = 0;
		for (ItemBlock *p = head; p; p = p->next)
			res += p->count;
		return res;
	}
};

#endif // __history_array__
