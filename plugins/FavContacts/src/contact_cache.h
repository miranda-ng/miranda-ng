#ifndef __contact_cache__
#define __contact_cache__

class CContactCache
{
public:
	enum { INFOSIZE = 1024 };

	struct TContactInfo
	{
		MCONTACT hContact;
		float rate;
		bool infoLoaded;
		TCHAR info[INFOSIZE];
	
		TContactInfo()
		{
			info[0] = info[1] = 0;
			infoLoaded = false;
		}

		void LoadInfo();
	};

private:
	LIST<TContactInfo> m_cache;
	unsigned long m_lastUpdate;
	CRITICAL_SECTION m_cs;

	int __cdecl OnDbEventAdded(WPARAM wParam, LPARAM lParam);
	float GetEventWeight(unsigned long age);
	float GetTimeWeight(unsigned long age);

public:
	CContactCache();
	~CContactCache();

	void Rebuild();

	MCONTACT get(int rate);
	float getWeight(int rate);
	bool filter(int rate, TCHAR *str);
};

#endif // __contact_cache__
