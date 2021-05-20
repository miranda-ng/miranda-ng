// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#undef _HAS_EXCEPTIONS
#define _HAS_EXCEPTIONS 1

#include <windows.h>
#include <mshtml.h>
#include <comdef.h>
#include <commctrl.h>
#include <ShellAPI.h>
#include <sys/stat.h>
#include <CommDlg.h>
#include <fstream>
#include <msapi/comptr.h>

#include <newpluginapi.h>
#include <m_clist.h>
#include <m_database.h>
#include <m_extraicons.h>
#include <m_genmenu.h>
#include <m_gui.h>
#include <m_icolib.h>
#include <m_json.h>
#include <m_langpack.h>
#include <m_netlib.h>
#include <m_options.h>
#include <m_popup.h>
#include <m_protosvc.h>
#include <m_userinfo.h>
#include <m_xml.h>

#include <m_variables.h>
#include <m_CurrencyRates.h>
#include <m_toptoolbar.h>

#include <boost\date_time\posix_time\posix_time.hpp>
#include <boost\date_time\c_local_time_adjustor.hpp>

typedef boost::posix_time::wtime_input_facet ttime_input_facet;
typedef boost::posix_time::wtime_facet ttime_facet;

#include "resource.h"
#include "version.h"
#include "CurrencyRateInfoDlg.h"
#include "ModuleInfo.h"
#include "DBUtils.h"
#include "HTTPSession.h"
#include "CurrencyConverter.h"
#include "WinCtrlHelper.h"
#include "ImportExport.h"
#include "CommonOptionDlg.h"
#include "EconomicRateInfo.h"
#include "SettingsDlg.h"
#include "Locale.h"
#include "ExtraImages.h"
#include "IsWithinAccuracy.h"
#include "ICurrencyRatesProvider.h"
#include "CurrencyRatesProviderBase.h"

#define CHART_IMPLEMENT
#ifdef CHART_IMPLEMENT
#include "CurrencyRateChart.h"
#include "Chart.h"
#endif

CMStringW CreateFilePath(const CMStringW &rsName);

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};
