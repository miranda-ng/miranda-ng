#ifndef _stopspam_headers_h
#define _stopspam_headers_h

// disable security warnings about "*_s" functions
#define _CRT_SECURE_NO_DEPRECATE

#include <time.h>

#include <windows.h>
#include <commctrl.h>
#include <sstream>
#include <list>
#include <map>

#include <newpluginapi.h>
#include <m_clistint.h>
#include <m_contacts.h>
#include <m_database.h>
#include <m_gui.h>
#include <m_icolib.h>
#include <m_langpack.h>
#include <m_netlib.h>
#include <m_options.h>
#include <m_protosvc.h>
#include <m_skin.h>

#include <m_variables.h>

typedef std::wstring tstring;
#define PREF_TCHAR2 PREF_UTF

#include "version.h"
#include "resource.h"

#define MODULENAME LPGEN("StopSpam")

#define DB_KEY_ANSWERED "Answered"
#define DB_KEY_HASAUTH "HasAuth"
#define DB_KEY_HASSENT "HasSent"
#define DB_KEY_QUESTCOUNT "QuestionCount"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	struct Impl
	{
		Impl() :
			timerAnswer(Miranda_GetSystemWindow(), UINT_PTR(this))
		{
			timerAnswer.OnEvent = Callback(this, &Impl::OnTimer);
		}

		CTimer timerAnswer;
		void OnTimer(CTimer *);
	} m_impl;

	CMOption<bool> bInfTalkProtection, bAddPermanent, bHandleAuthReq, bAnswNotCaseSens, bHistLog;
	CMOption<wchar_t *> Question, AuthRepl, Answer, Congratulation, AnswSplitString;
	CMOption<char *> DisabledProtoList;
	CMOption<uint32_t> iMaxQuestCount, iAnswerTimeout;

	const wchar_t* getQuestion();
	const wchar_t* getReply();
	const wchar_t* getCongrats();

	bool ProtoDisabled(const char *proto)
	{
		std::string temp(proto); temp += ' ';
		return strstr(DisabledProtoList, temp.c_str()) != nullptr;
	}

	int Load() override;
	int Unload() override;
};

// utils
tstring variables_parse(const wchar_t *tstrFormat, MCONTACT hContact);
tstring trim(tstring const &tstr, tstring const &trimChars = L" \f\n\r\t\v");

INT_PTR IsContactPassed(WPARAM, LPARAM);
int OnDbEventAdded(WPARAM, LPARAM);
int OnDbEventFilterAdd(WPARAM, LPARAM);
int OnOptInit(WPARAM, LPARAM);
int OnShutdown(WPARAM, LPARAM);

#endif
