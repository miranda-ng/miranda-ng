#include "StdAfx.h"
#include "ExtraImages.h"
#include "IconLib.h"
#include "EconomicRateInfo.h"
#include "ModuleInfo.h"
#include "QuotesProviders.h"
#include "IQuotesProvider.h"
#include "Log.h"
#include "DBUtils.h"

namespace
{
	inline HANDLE extra_add_icon(const char* psz)
	{
		return reinterpret_cast<HANDLE>(
			CallService(MS_CLIST_EXTRA_ADD_ICON,reinterpret_cast<WPARAM>(Quotes_LoadIconEx(psz)),0));
	}

	const HANDLE INVALID_IMAGE_HANDLE = reinterpret_cast<HANDLE>(-1);
}

CExtraImages::CExtraImages()
			 : m_hExtraIcons(ExtraIcon_Register(ICON_STR_QUOTE,QUOTES_PROTOCOL_NAME,Quotes_MakeIconName(ICON_STR_MAIN).c_str())),
			   m_bExtraImagesInit(false),
			   m_nSlot(DBGetContactSettingWord(NULL,QUOTES_PROTOCOL_NAME,DB_STR_QUOTE_EXTRA_IMAGE_SLOT,EXTRA_ICON_ADV1))
{
	m_ahExtraImages[eiUp] = INVALID_IMAGE_HANDLE;
	m_ahExtraImages[eiDown] = INVALID_IMAGE_HANDLE;
	m_ahExtraImages[eiNotChanged] = INVALID_IMAGE_HANDLE;
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
	if(NULL == m_hExtraIcons)
	{
		m_bExtraImagesInit = false;
		CGuard<CLightMutex> cs(m_lmExtraImages);
		m_ahExtraImages[eiUp] = extra_add_icon(ICON_STR_QUOTE_UP);
		m_ahExtraImages[eiDown] = extra_add_icon(ICON_STR_QUOTE_DOWN);
		m_ahExtraImages[eiNotChanged] = extra_add_icon(ICON_STR_QUOTE_NOT_CHANGED);
		m_bExtraImagesInit = true;
	}
}


bool CExtraImages::SetContactExtraImage(HANDLE hContact,EImageIndex nIndex)const
{
// 	tstring s = Quotes_DBGetStringT(hContact,LIST_MODULE_NAME,CONTACT_LIST_NAME);
// 	tostringstream o;
// 	o << _T("SetContactExtraImage for ") << s << _T("\nExtra image list init: ") << m_bExtraImagesInit << _T("\n");
	
	bool bResult = false;
	if(m_hExtraIcons)
	{
// 		o << "Using extra icon interface\n";
		std::string sIconName;
		switch(nIndex)
		{
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
		bResult = (0 == ExtraIcon_SetIcon(m_hExtraIcons,hContact,sIconName.c_str()));
	}
	else if(m_bExtraImagesInit && ServiceExists(MS_CLIST_EXTRA_ADD_ICON))
	{
// 		o << "Using contact list interface index is ";
		IconExtraColumn iec = {0};
		iec.cbSize = sizeof(iec);
		iec.ColumnType = m_nSlot;

		{
			CGuard<CLightMutex> cs(m_lmExtraImages);
			switch(nIndex)
			{
			case eiUp:
// 				o << "up\n";
				iec.hImage = m_ahExtraImages[eiUp];
				break;
			case eiDown:
// 				o << "down\n";
				iec.hImage = m_ahExtraImages[eiDown];
				break;
			case eiNotChanged:
// 				o << "not changed\n";
				iec.hImage = m_ahExtraImages[eiNotChanged];
				break;
			default:
// 				o << "invalid\n";
				iec.hImage = INVALID_IMAGE_HANDLE;
				break;
			}
		}

		bResult = (0 == CallService(MS_CLIST_EXTRA_SET_ICON,reinterpret_cast<WPARAM>(hContact),reinterpret_cast<LPARAM>(&iec)));
	}

// 	o << "Result is " << bResult;
// 	LogIt(Info,o.str());
	return bResult;
}

int QuotesEventFunc_onExtraImageListRebuild(WPARAM /*wp*/,LPARAM /*lp*/)
{
	if (ServiceExists(MS_CLIST_EXTRA_ADD_ICON))
	{
		CExtraImages::GetInstance().RebuildExtraImages();
	}

	return 0;
}

int QuotesEventFunc_onExtraImageApply(WPARAM wp,LPARAM lp)
{
	HANDLE hContact = reinterpret_cast<HANDLE>(wp);

	const CModuleInfo::TQuotesProvidersPtr& pProviders = CModuleInfo::GetQuoteProvidersPtr();
	CQuotesProviders::TQuotesProviderPtr pProvider = pProviders->GetContactProviderPtr(hContact);
	if(pProvider)
	{
		pProvider->SetContactExtraIcon(hContact);
	}

	return 0;
}


