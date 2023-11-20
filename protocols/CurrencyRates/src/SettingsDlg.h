#ifndef __E211E4D9_383C_43BE_A787_7EF1D585B90D_SettingsDlg_h__
#define __E211E4D9_383C_43BE_A787_7EF1D585B90D_SettingsDlg_h__

void ShowSettingsDlg(MCONTACT hContact);

enum
{
	glfnResolveCurrencyRateName = 0x0001,
	glfnResolveUserProfile = 0x0002,
	glfnResolveAll = glfnResolveCurrencyRateName | glfnResolveUserProfile,
};

CMStringW GenerateLogFileName(const CMStringW &rsLogFilePattern, const CMStringW &rsCurrencyRateSymbol, int nFlags = glfnResolveAll);
CMStringW GetContactLogFileName(MCONTACT hContact);
CMStringW GetContactName(MCONTACT hContact);

#endif //__E211E4D9_383C_43BE_A787_7EF1D585B90D_SettingsDlg_h__

