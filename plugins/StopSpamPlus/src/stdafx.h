#ifndef _stopspam_headers_h
#define _stopspam_headers_h

// disable security warnings about "*_s" functions
#define _CRT_SECURE_NO_DEPRECATE

#include <windows.h>
#include <commctrl.h>
#include <sstream>
#include <list>

#include <newpluginapi.h>
#include <m_contacts.h>
#include <m_database.h>
#include <m_protosvc.h>
#include <m_options.h>
#include <m_langpack.h>
#include <m_icolib.h>
#include <m_skin.h>
#include <m_clistint.h>
#include <m_gui.h>

#include <m_stopspam.h>
#include <m_variables.h>

typedef std::wstring tstring;
#define PREF_TCHAR2 PREF_UTF

#include "version.h"
#include "resource.h"

#define MODULENAME LPGEN("StopSpam")

#define DB_KEY_ANSWERED "Answered"
#define DB_KEY_QUESTCOUNT "QuestionCount"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	CMOption<bool> InfTalkProtection, AddPermanent, HandleAuthReq, AnswNotCaseSens, HistLog;
	CMOption<wchar_t *> Question, AuthRepl, Answer, Congratulation, AnswSplitString;
	CMOption<char *> DisabledProtoList;
	CMOption<uint32_t> MaxQuestCount;

	const wchar_t* getQuestion();
	const wchar_t* getReply();
	const wchar_t* getCongrats();

	bool ProtoDisabled(const char *proto)
	{
		std::string temp(proto); temp += ' ';
		return strstr(DisabledProtoList, temp.c_str()) != nullptr;
	}

	int Load() override;
};

// utils
tstring &GetDlgItemString(HWND hwnd, int id);
bool IsExistMyMessage(MCONTACT hContact);
tstring variables_parse(const wchar_t *tstrFormat, MCONTACT hContact);
tstring trim(tstring const &tstr, tstring const &trimChars = L" \f\n\r\t\v");

INT_PTR IsContactPassed(WPARAM wParam, LPARAM /*lParam*/);
int OnDbEventAdded(WPARAM wParam, LPARAM lParam);
int OnDbEventFilterAdd(WPARAM w, LPARAM l);
int OnOptInit(WPARAM w, LPARAM l);
int OnDbContactSettingchanged(WPARAM hContact, LPARAM l);
#endif
