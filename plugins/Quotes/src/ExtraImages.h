#ifndef __9d0dac0c_12e4_46ce_809a_db6dc7d6f269_ExtraImages_h__
#define __9d0dac0c_12e4_46ce_809a_db6dc7d6f269_ExtraImages_h__

class CExtraImages : private boost::noncopyable
{
public:
	enum EImageIndex
	{
		eiUp = 0,
		eiDown = 1,
		eiNotChanged = 2,
		eiEmpty = 3,
		ImageCount = 3
	};

private:
	CExtraImages();
	~CExtraImages();

public:
	static CExtraImages& GetInstance();

	void RebuildExtraImages();
	bool SetContactExtraImage(MCONTACT hContact,EImageIndex nIndex)const;

private:
	mutable CLightMutex m_lmExtraImages;
	HANDLE m_ahExtraImages[ImageCount];
	HANDLE m_hExtraIcons;
	bool m_bExtraImagesInit;
};

// int QuotesEventFunc_onExtraImageApply(WPARAM wp,LPARAM lp);

#endif //__9d0dac0c_12e4_46ce_809a_db6dc7d6f269_ExtraImages_h__
