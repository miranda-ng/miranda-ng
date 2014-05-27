#include "StdAfx.h"

CExtraImages::CExtraImages() :
	m_hExtraIcons(ExtraIcon_Register(ICON_STR_QUOTE,QUOTES_PROTOCOL_NAME,Quotes_MakeIconName(ICON_STR_MAIN).c_str())),
	m_bExtraImagesInit(false)
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

bool CExtraImages::SetContactExtraImage(MCONTACT hContact,EImageIndex nIndex)const
{
	if (!m_hExtraIcons)
		return false;

	HANDLE hIcolib;
	switch(nIndex) {
	case eiUp:
		hIcolib = Quotes_GetIconHandle(IDI_ICON_UP);
		break;
	case eiDown:
		hIcolib = Quotes_GetIconHandle(IDI_ICON_DOWN);
		break;
	case eiNotChanged:
		hIcolib = Quotes_GetIconHandle(IDI_ICON_NOTCHANGED);
		break;
	default:
		hIcolib = NULL;
	}
	return ExtraIcon_SetIcon(m_hExtraIcons, hContact, hIcolib) == 0;
}

// int QuotesEventFunc_onExtraImageApply(WPARAM wp,LPARAM lp)
// {
// 	MCONTACT hContact = MCONTACT(wp);
// 
// 	const CModuleInfo::TQuotesProvidersPtr& pProviders = CModuleInfo::GetQuoteProvidersPtr();
// 	CQuotesProviders::TQuotesProviderPtr pProvider = pProviders->GetContactProviderPtr(hContact);
// 	if(pProvider)
// 		pProvider->SetContactExtraIcon(hContact);
// 
// 	return 0;
// }
