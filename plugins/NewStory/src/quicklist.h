template <class T>
class QuickList
{
private:
	struct Item
	{
		T data;
		Item* prev, * next;
	};
	struct CacheItem
	{
		CacheItem() : item(0), no(0) {}
		Item* item;
		int no;
	};
	Item* head, * tail;
	int cacheSize, cacheStep, cacheUsed;
	CacheItem* cache;
	CacheItem last;

public:
	QuickList(int theCacheSize = 100, int theCacheStep = )
	{
		head = tail = 0;
		cacheSize = theCacheSize;
		cacheStep = theCacheStep;
		cacheUsed = 0;
		cache = new CacheItem[cacheSize];
	}
	~QuickList()
	{
		delete[] cache;
	}
};
