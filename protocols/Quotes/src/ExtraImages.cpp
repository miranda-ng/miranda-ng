#include "StdAfx.h"
#include "ExtraImages.h"
#include "IconLib.h"
#include "EconomicRateInfo.h"
#include "ModuleInfo.h"
#include "QuotesProviders.h"
#include "IQuotesProvider.h"
#include "Log.h"
#include "DBUtils.h"

CExtraImages::CExtraImages() :
	m_hExtraIcons(ExtraIcon_Register(ICON_STR_QUOTE,QUOTES_PROTOCOL_NAME,Quotes_MakeIconName(ICON_STR_MAIN).c_str())),
	m_bExtraImagesInit(false),
	m_nSlot(DBGetContactSettingWord(NULL,QUOTES_PROTOCOL_NAME,DB_STR_QUOTE_EXTRA_IMAGE_SLOT,EXTRA_ICON_ADV1))
{
	m_ahExtraImages[eiUp] = INVALID_HANDLE_VALUE;
	m_ahExtraImages[eiDown] = INVALID_HANDLE_VALUE;
	m_ahExtraImages[eiNotChanged] = INVALID_HANDLE_VALUE;
}

CExtraImages::~CExtraImages()
{
}

CExtraImages& CExtraImages::GetInstance()
{
	static CExtraImages s_ei;
	return s_ei;
}

void CExtraImages::RebuildExtraImages()
{
}

bool CExtraImages::SetContactExtraImage(HANDLE hContact,EImageIndex nIndex)const
{
	if (!m_hExtraIcons)
		return false;

	std::string sIconName;
	switch(nIndex) {
	case eiUp:
		sIconName = Quotes_MakeIconName(ICON_STR_QUOTE_UP);
		break;
	case eiDown:
		sIconName = Quotes_MakeIconName(ICON_STR_QUOTE_DOWN);
		break;
	case eiNotChanged:
		sIconName = Quotes_MakeIconName(ICON_STR_QUOTE_NOT_CHANGED);
		break;
	}
	return (0 == ExtraIcon_SetIcon(m_hExtraIcons,hContact,sIconName.c_str()));
}

int QuotesEventFunc_onExtraImageApply(WPARAM wp,LPARAM lp)
{
	HANDLE hContact = reinterpret_cast<HANDLE>(wp);

	const CModuleInfo::TQuotesProvidersPtr& pProviders = CModuleInfo::GetQuoteProvidersPtr();
	CQuotesProviders::TQuotesProviderPtr pProvider = pProviders->GetContactProviderPtr(hContact);
	if(pProvider)
		pProvider->SetContactExtraIcon(hContact);

	return 0;
}
