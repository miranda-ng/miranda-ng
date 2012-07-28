/*
 *  Smart Auto Replier (SAR) - auto replier plugin for Miranda IM
 *
 *  Copyright (C) 2004 - 2012 by Volodymyr M. Shcherbyna <volodymyr@shcherbyna.com>
 *
 *      This file is part of SAR.
 *
 *  SAR is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  SAR is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with SAR.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#include "settingshandler.h"
#include "modemesshandler.h"

/// we need hinst
extern HINSTANCE hInst;
extern CCrushLog CRUSHLOGOBJ;
extern INT	     g_nCurrentMode;

/// ctor
CSettingsHandler::CSettingsHandler(void) : m_bResettings(false),
m_replSize(sizeof(REPLYER_SETTINGS))
{
BEGIN_PROTECT_AND_LOG_CODE
	MakeFullPath(m_szSettFileName, sizeof(m_szSettFileName), SETTINGS_DB);
	m_rulesStorage.Init();
END_PROTECT_AND_LOG_CODE
}

/// d'ctor
CSettingsHandler::~CSettingsHandler(void)
{
BEGIN_PROTECT_AND_LOG_CODE
	DeInit();
	m_rulesStorage.Flush();
END_PROTECT_AND_LOG_CODE
}

/// changing settings...
void CSettingsHandler::setSettings(REPLYER_SETTINGS & settings, COMMON_RULE_ITEM * pcommrule)
{
BEGIN_PROTECT_AND_LOG_CODE
	m_Settings = settings;
#ifdef _DEBUG
	bool bflushed =
#endif
	Serialize(&m_Settings,  m_replSize);
	if (pcommrule)
	{
		m_rulesStorage.setCommonRule(*pcommrule);
		DWORD dw = NULL;
		m_rulesStorage.Serialize(NULL, dw); 
	}
END_PROTECT_AND_LOG_CODE
}

bool CSettingsHandler::Serialize(ISettingsStream *pSettings, DWORD & ObjectSize)
{
BEGIN_PROTECT_AND_LOG_CODE
	bool bRetVal = false;
	HANDLE hFile = CreateFile (m_szSettFileName, GENERIC_WRITE,
							   FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
							   FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)
		return bRetVal; ///
	else
	{	/// check ptr for null
		bool bValid = (pSettings != NULL);
		if (bValid) /// for readibility..
			bValid = (IsBadReadPtr(pSettings, sizeof(pSettings)) == TRUE ? false : true);
		
		DWORD dwWritten = NULL;
		bRetVal = (WriteFile (hFile, (bValid ? pSettings : &m_Settings), sizeof(REPLYER_SETTINGS), &dwWritten, NULL) == TRUE ? true : false);
		CloseHandle (hFile);
		return bRetVal;
	}
END_PROTECT_AND_LOG_CODE
	return false;
}
 
bool CSettingsHandler::Deserialize(DWORD & ObjectSize)
{
BEGIN_PROTECT_AND_LOG_CODE
	bool bRetVal = false;
	HANDLE hFile = CreateFile (m_szSettFileName, GENERIC_READ,
							   FILE_SHARE_READ, NULL, OPEN_EXISTING,
							   FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)
		return bRetVal; ///
	else
	{
		DWORD dwSizeHigh = 0;
		DWORD dwSize = GetFileSize(hFile, &dwSizeHigh);

		if (dwSize != ObjectSize)
		{
			bool bDel = NotifyAboutWrongSettings(m_szSettFileName);
			if (bDel)
			{
				CloseHandle (hFile);
				DeleteFile (m_szSettFileName);
				m_bResettings = true;
				Init();
				m_bResettings = false;
				return true;
			}
		}
		DWORD dwReaded = NULL;
		bRetVal = (ReadFile (hFile, &m_Settings, ObjectSize, &dwReaded, NULL) == TRUE ? true : false);
		CloseHandle (hFile);

		return bRetVal;
	}
END_PROTECT_AND_LOG_CODE
	return false;
}

void CSettingsHandler::Init(void)
{
BEGIN_PROTECT_AND_LOG_CODE

	if (!PathFileExists(m_szSettFileName))
	{	/// create file with def settings...		
		/// create def. configuration and store it to file...
		m_Settings.bDisableWhenModeIsSet = SETTINGS_DEF_COMMON_EN_DISONMODEON;
		m_Settings.ModeDisValue = SETTINGS_DEF_COMMON_EN_ONMODEDISVAL;
		m_Settings.bShowAURDlgWhenModeChanges = false;
		m_Settings.bEnableWhenModeIsSet = SETTINGS_DEF_COMMON_EN_ONMODEON;
		m_Settings.ModeValue = SETTINGS_DEF_COMMON_EN_ONMODEONVAL;

		m_Settings.bDisableWhenMirandaIsOn = SETTINGS_DEF_COMMON_DWMION;
		m_Settings.ReplayDelay = SETTINGS_DEF_COMMON_VALREPDELAY;
		m_Settings.bEnabled = true;
		m_Settings.bFirstRun  = !m_bResettings;
		m_Settings.bSaveToHistory = false;

		Serialize(&m_Settings, m_replSize);

		if (m_Settings.bFirstRun)
		{ /// show only once to user readme file...
			m_Settings.bFirstRun = false;
			TCHAR * szreadme = NULL;
			szreadme = new TCHAR[MAX_PATH + 1];
			if (szreadme)
			{			
				//MakeFullPath(szreadme, MAX_PATH, READMEFILE);
				::ShellExecute(NULL, TEXT("open"), TEXT("http://www.shcherbyna.com/?page_id=1747"), NULL, NULL, SW_SHOW);
				//::ShellExecute(NULL, "open", TEXT("http://shcherbyna.com/forum/viewforum.php?f=8"), NULL, NULL, SW_SHOW);

				delete szreadme;
			}
		}
	}
	else
	{
		Deserialize(m_replSize);
	}
END_PROTECT_AND_LOG_CODE
}

void CSettingsHandler::DeInit(void)
{
BEGIN_PROTECT_AND_LOG_CODE
	Serialize(&m_Settings, m_replSize);
END_PROTECT_AND_LOG_CODE
}

CScriptsReader & CSettingsHandler::getScriptsReader(void)
{
	return m_scriptsReader;
}

REPLYER_SETTINGS & CSettingsHandler::getSettings(void)
{
BEGIN_PROTECT_AND_LOG_CODE
	return m_Settings;
END_PROTECT_AND_LOG_CODE
}

DWORD CSettingsHandler::AddReplyAction(RULE_ITEM item, bool & bExists)
{
BEGIN_PROTECT_AND_LOG_CODE
	return m_rulesStorage.AddRuleItem(item, bExists);
END_PROTECT_AND_LOG_CODE
	return 0;
}

CRulesStorage & CSettingsHandler::getStorage(void)
{
BEGIN_PROTECT_AND_LOG_CODE
	return m_rulesStorage;
END_PROTECT_AND_LOG_CODE
}