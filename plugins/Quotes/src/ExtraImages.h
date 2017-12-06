#ifndef __9d0dac0c_12e4_46ce_809a_db6dc7d6f269_ExtraImages_h__
#define __9d0dac0c_12e4_46ce_809a_db6dc7d6f269_ExtraImages_h__

enum EImageIndex
{
	eiUp = 0,
	eiDown = 1,
	eiNotChanged = 2,
	eiEmpty = 3
};

bool SetContactExtraImage(MCONTACT hContact, EImageIndex nIndex);

void Quotes_InitExtraIcons(void);

#endif //__9d0dac0c_12e4_46ce_809a_db6dc7d6f269_ExtraImages_h__
