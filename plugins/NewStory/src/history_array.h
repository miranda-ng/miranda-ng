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
		BYTE flags = 0;

		MCONTACT hContact = 0;
		MEVENT hEvent = 0;

		bool dbeOk = false;
		DBEVENTINFO dbe;

		bool wtext_del = false;
		wchar_t* wtext = 0;

		HANDLE data = 0;

		ItemData() { memset(&dbe, 0, sizeof(dbe)); }
		~ItemData();

		bool load(EventLoadMode mode);
		inline bool loadInline(EventLoadMode mode)
		{
			if (((mode >= ELM_INFO) && !dbeOk) || ((mode == ELM_DATA) && !dbe.pBlob))
				return load(mode);
			return true;
		}
		inline wchar_t* getWBuf()
		{
			loadInline(ELM_DATA);
			return wtext;
		}
	};

	struct ItemBlock
	{
		ItemData* items;
		int count;
		ItemBlock* prev, * next;
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
			STATUS = 0x020,
			OTHER = 0x040,
			EVENTTEXT = 0x080,
			EVENTONLY = 0x100,
		};
		Filter(WORD aFlags, wchar_t* wText)
		{
			refCount = new int(0);
			flags = aFlags;
			text = new wchar_t[mir_wstrlen(wText) + 1];
			mir_wstrcpy(text, wText);
		}
		Filter(const Filter& other)
		{
			flags = other.flags;
			refCount = other.refCount;
			text = other.text;
			++* refCount;
		}
		Filter& operator=(const Filter& other)
		{
			flags = other.flags;
			refCount = other.refCount;
			text = other.text;
			++* refCount;
		}
		~Filter()
		{
			if (!-- * refCount) {
				delete refCount;
				if (text) delete[] text;
			}
		}
		inline bool check(ItemData* item)
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
				return CheckFilter(item->getWBuf(), text);
			}
			return true;
		};

	private:
		WORD flags;
		int* refCount;
		wchar_t* text;
	};

private:
	ItemBlock* head = 0, * tail = 0;
	ItemBlock* preBlock = 0;
	int preIndex = 0;
	bool allocateBlock(int count);

public:
	HistoryArray();
	~HistoryArray();

	void clear();
	bool addHistory(MCONTACT hContact, EventLoadMode mode = ELM_NOTHING);
	bool addEvent(MCONTACT hContact, MEVENT hEvent, int count, EventLoadMode mode = ELM_NOTHING);

	//	bool preloadEvents(int count = 10);

	ItemData* get(int id, EventLoadMode mode = ELM_NOTHING);
	ItemData* operator[] (int id) { return get(id, ELM_DATA); }
	ItemData* operator() (int id) { return get(id, ELM_INFO); }

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
		for (ItemBlock* p = head; p; p = p->next)
			res += p->count;
		return res;
	}
};

#endif // __history_array__
