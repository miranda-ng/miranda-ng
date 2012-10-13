#ifndef __a33da2bb_d4fe4aa7_aaf5_f9f8c3156ce6_LightMutex_h__
#define __a33da2bb_d4fe4aa7_aaf5_f9f8c3156ce6_LightMutex_h__

class CLightMutex
{
public:
	CLightMutex();
	~CLightMutex();

	void Lock();
	void Unlock();

private:
	CRITICAL_SECTION m_cs;
};

template<class TObject> class CGuard
{
public:
	CGuard(TObject& obj) : m_obj(obj)
	{
		m_obj.Lock();
	}

	~CGuard()
	{
		m_obj.Unlock();
	}

private:
	TObject& m_obj;
};

#endif //__a33da2bb_d4fe4aa7_aaf5_f9f8c3156ce6_LightMutex_h__
