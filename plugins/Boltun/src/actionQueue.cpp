//***********************************************************
//	Copyright  2003-2008 Alexander S. Kiselev, Valentin Pavlyuchenko
//
//	This file is part of Boltun.
//
//    Boltun is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 2 of the License, or
//    (at your option) any later version.
//
//    Boltun is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//	  along with Boltun. If not, see <http://www.gnu.org/licenses/>.
//
//***********************************************************

#include "stdafx.h"

#include "newpluginapi.h"
#include "m_database.h"
#include "m_system.h"
#include "m_protosvc.h"

using namespace std;

extern TalkBot* bot;

typedef void(*ActionHandler)(MCONTACT hContact, const TalkBot::MessageInfo *inf);

typedef struct _QueueElement {
	MCONTACT hContact;
	const TalkBot::MessageInfo *inf;
	ActionHandler Handler;
	bool Sticky;
	int TimeOffset;
	_QueueElement(MCONTACT contact, ActionHandler handler, int timeOffset, const TalkBot::MessageInfo *info = NULL, bool sticky = false)
		:hContact(contact), Handler(handler), TimeOffset(timeOffset), inf(info), Sticky(sticky)
	{
	}
} QueueElement;

static list<QueueElement> actionQueue;
static set<MCONTACT> typingContacts;
UINT_PTR timerID = 0;

mir_cs cs;
mir_cs typingContactsLock;

void UpdateTimer();

VOID CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD)
{
	mir_cslockfull lck(cs);
	QueueElement q = actionQueue.front();
	actionQueue.pop_front();
	UpdateTimer();
	lck.unlock();

	q.Handler(q.hContact, q.inf);
}

void UpdateTimer()
{
	if (timerID)
		KillTimer(NULL, timerID);
	if (actionQueue.size())
		timerID = SetTimer(NULL, 0, actionQueue.front().TimeOffset, TimerProc);
	else
		timerID = 0;
}

static bool NotifyTyping(MCONTACT hContact)
{
	int res = db_get_b(hContact, "SRMsg", "SupportTyping", 2);
	if (res == 2)
		res = db_get_b(NULL, "SRMsg", "DefaultTyping", 1);
	return res != 0;
}

static void TimerAnswer(MCONTACT hContact, const TalkBot::MessageInfo* info)
{
	ptrA msg(mir_utf8encodeT(info->Answer.c_str()));
	size_t bufsize = mir_strlen(msg);

	CallContactService(hContact, PSS_MESSAGE, 0, (LPARAM)msg);

	DBEVENTINFO dbei = { 0 };
	dbei.cbSize = sizeof(dbei);
	dbei.cbBlob = (int)bufsize;
	dbei.pBlob = (PBYTE)(char*)msg;
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.flags = DBEF_SENT;
	dbei.szModule = BOLTUN_NAME;
	dbei.timestamp = (DWORD)time(NULL);

	db_event_add(hContact, &dbei);
	bot->AnswerGiven(hContact, *info);
	delete info;

	delete[] msg;

	mir_cslock lck(typingContactsLock);
	typingContacts.erase(hContact);
}

static void StartTyping(MCONTACT hContact, const TalkBot::MessageInfo*)
{
	CallService(MS_PROTO_SELFISTYPING, hContact, PROTOTYPE_SELFTYPING_ON);

	mir_cslock lck(typingContactsLock);
	typingContacts.insert(hContact);
}

void DoAnswer(MCONTACT hContact, const TalkBot::MessageInfo *info, bool sticky = false)
{
	if (info->Answer[0] == _T('\0'))
		return;
	int waitTime, thinkTime = 0;
	int defWaitTime = Config.AnswerPauseTime * 1000;
	if (Config.PauseDepends)
		waitTime = defWaitTime * (int)info->Answer.length() / 25;
	else
		waitTime = defWaitTime;
	if (Config.PauseRandom)
	{
		//Let it be up to 4 times longer.
		waitTime = waitTime * (rand() % 300) / 100 + waitTime;
	}
	if (waitTime == 0)
		waitTime = 50; //it's essential, because otherwise message will be added later 
	//then its response, that will cause incorrect ordering of 
	//messages in the opened history (reopening will
	//help, but anyway it's no good)
	if (NotifyTyping(hContact) && Config.AnswerThinkTime)
	{
		thinkTime = Config.AnswerThinkTime * 1000;
		if (Config.PauseRandom)
		{
			//Let it be up to 4 times longer.
			thinkTime = thinkTime * (rand() % 300) / 100 + thinkTime;
		}
	}
	
	mir_cslock lck(cs);
	//Check if this contact's timer handler is now waiting for a cs.
	bool needTimerRearrange = false;
	if (!actionQueue.empty() && actionQueue.front().hContact == hContact)
	{
		needTimerRearrange = true;
		KillTimer(NULL, timerID);
	}
	if (!actionQueue.empty())
	{
		list<QueueElement>::iterator it = actionQueue.end();
		--it;
		while (true)
		{
			if ((*it).hContact == hContact)
			{
				if ((*it).Sticky)
					break;
				list<QueueElement>::iterator tmp = it;
				if (tmp != actionQueue.begin())
					--tmp;
				actionQueue.erase(it);
				it = tmp;
				if (actionQueue.empty())
					break;
			}
			if (it == actionQueue.begin())
				break;
			--it;
		}
	}
	{
		mir_cslock tcl(typingContactsLock);
		if (typingContacts.find(hContact) != typingContacts.end())
		{
			CallService(MS_PROTO_SELFISTYPING, hContact, (LPARAM)PROTOTYPE_SELFTYPING_OFF);
			typingContacts.erase(hContact);
		}
	}
	if (actionQueue.empty())
		needTimerRearrange = true;
	if (thinkTime)
		actionQueue.push_back(QueueElement(hContact, StartTyping, thinkTime, NULL, sticky));
	actionQueue.push_back(QueueElement(hContact, TimerAnswer, waitTime, info, sticky));
	if (needTimerRearrange)
		UpdateTimer();
}

void AnswerToContact(MCONTACT hContact, const TCHAR* messageToAnswer)
{
	if (Config.TalkWarnContacts && db_get_b(hContact, BOLTUN_KEY,
		DB_CONTACT_WARNED, FALSE) == FALSE)
	{
		DoAnswer(hContact, new TalkBot::MessageInfo((const TCHAR*)Config.WarnText), true);
		db_set_b(hContact, BOLTUN_KEY, DB_CONTACT_WARNED, TRUE);
	}
	else
		DoAnswer(hContact, bot->Reply(hContact, messageToAnswer, false));
}

void StartChatting(MCONTACT hContact)
{
	DoAnswer(hContact, new TalkBot::MessageInfo(bot->GetInitMessage(hContact)), true);
}
