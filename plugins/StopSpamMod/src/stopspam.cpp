/* Copyright (C) Miklashevsky Roman, sss, elzor
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/


#include "headers.h"

MIRANDA_HOOK_EVENT(ME_DB_CONTACT_ADDED, w, l)
{
	return 0;
}

MIRANDA_HOOK_EVENT(ME_DB_EVENT_ADDED, hContact, hDbEvent)
{
	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.cbBlob = db_event_getBlobSize(hDbEvent);
	if (dbei.cbBlob == -1)
		return 0;
	
	dbei.pBlob = (BYTE*)alloca(dbei.cbBlob);
	db_event_get(hDbEvent, &dbei);

	// if event is in protocol that is not despammed
	if(!ProtoInList(dbei.szModule))
		return 0;

	// event is an auth request
	if(gbHandleAuthReq) {
		if(!(dbei.flags & DBEF_SENT) && !(dbei.flags & DBEF_READ) && dbei.eventType == EVENTTYPE_AUTHREQUEST) {
			MCONTACT hcntct = DbGetAuthEventContact(&dbei);

			// if request is from unknown or not marked Answered contact
			int a = db_get_b(hcntct, "CList", "NotOnList", 0);
			int b = !db_get_b(hcntct, pluginName, "Answered", 0);
			
			if(a && b) {
				// ...send message

				if(gbHideContacts)
					db_set_b(hcntct, "CList", "Hidden", 1);
				if(gbSpecialGroup)
					db_set_ws(hcntct, "CList", "Group", gbSpammersGroup.c_str());
				BYTE msg = 1;
				if(gbIgnoreURL){
					TCHAR* EventText = ReqGetText(&dbei); //else return NULL
					msg=!IsUrlContains(EventText);
					mir_free(EventText);
				}
				if(gbInvisDisable) {
					if(CallProtoService(dbei.szModule, PS_GETSTATUS, 0, 0) == ID_STATUS_INVISIBLE)
						msg = 0;
					else if(db_get_w(hContact,dbei.szModule,"ApparentMode",0) == ID_STATUS_OFFLINE)
						msg = 0; //is it useful ?
				}
				if(msg) {
					char * buff=mir_utf8encodeW(variables_parse(gbAuthRepl, hcntct).c_str());
					CallContactService(hcntct, PSS_MESSAGE, PREF_UTF, (LPARAM) buff);
					mir_free(buff);
				}
				return 1;
			}
		}
	}
	return 0;
}

MIRANDA_HOOK_EVENT(ME_DB_EVENT_FILTER_ADD, w, l)
{
	MCONTACT hContact = (MCONTACT)w;
	if(!l) //fix potential DEP crash
		return 0;
	DBEVENTINFO * dbei = (DBEVENTINFO*)l;
	
	// if event is in protocol that is not despammed
	if(!ProtoInList(dbei->szModule))
	{
		// ...let the event go its way
		return 0;
	}
	//do not check excluded contact

	if(db_get_b(hContact, pluginName, "Answered", 0))
		return 0;
	if(db_get_b(hContact, pluginName, "Excluded", 0))
	{
		if(!db_get_b(hContact, "CList", "NotOnList", 0))
			db_unset(hContact, pluginName, "Excluded");
		return 0;
	}
	//we want block not only messages, i seen many types other eventtype flood
	if(dbei->flags & DBEF_READ)
		// ...let the event go its way
		return 0;
	//mark contact which we trying to contact for exclude from check
	if((dbei->flags & DBEF_SENT) && db_get_b(hContact, "CList", "NotOnList", 0)
		&& (!gbMaxQuestCount || db_get_dw(hContact, pluginName, "QuestionCount", 0) < gbMaxQuestCount) && gbExclude)
	{
		db_set_b(hContact, pluginName, "Excluded", 1);
		return 0;
	}
	// if message is from known or marked Answered contact
	if(!db_get_b(hContact, "CList", "NotOnList", 0))
		// ...let the event go its way
		return 0;
	// if message is corrupted or empty it cannot be an answer.
	if(!dbei->cbBlob || !dbei->pBlob)
		// reject processing of the event
		return 1;

	tstring message;
	
	if(dbei->flags & DBEF_UTF)
	{
		wchar_t* msg_u;
		char* msg_a = mir_strdup(( char* )dbei->pBlob );
		mir_utf8decode( msg_a, &msg_u );
		message = msg_u;
	}
	else
		message = mir_a2u((char*)(dbei->pBlob));

	// if message contains right answer...

	boost::algorithm::erase_all(message, "\r");
	boost::algorithm::erase_all(message, "\n");
	
	BYTE msg = 1;
	if(gbInvisDisable)
	{
		if(CallProtoService(dbei->szModule, PS_GETSTATUS, 0, 0) == ID_STATUS_INVISIBLE)
			msg = 0;
		else if(db_get_w(hContact,dbei->szModule,"ApparentMode",0) == ID_STATUS_OFFLINE)
			msg = 0; //is it useful ?
	}
	bool answered = false;
	if(gbMathExpression)
	{
		if(boost::algorithm::all(message, boost::is_digit()))
		{
			int msg = _ttoi(message.c_str());
			int math_answer = db_get_dw(hContact, pluginName, "MathAnswer", 0);
			if(msg && math_answer)
				answered = (msg == math_answer);
		}
	}
	else if(!gbRegexMatch)
		answered = gbCaseInsensitive?(!Stricmp(message.c_str(), (variables_parse(gbAnswer, hContact).c_str()))):( !_tcscmp(message.c_str(), (variables_parse(gbAnswer, hContact).c_str())));
	else
	{
		if(gbCaseInsensitive)
		{
			std::string check(toUTF8(variables_parse(gbAnswer, hContact))), msg(toUTF8(message));
			boost::algorithm::to_upper(check);
			boost::algorithm::to_upper(msg);
			boost::regex expr(check);
			answered = boost::regex_search(msg.begin(), msg.end(), expr);
		}
		else
		{
			std::string check(toUTF8(variables_parse(gbAnswer, hContact))), msg(toUTF8(message));
			boost::regex expr(check);
			answered = boost::regex_search(msg.begin(), msg.end(), expr);
		}
	}
	if(answered)
	{
		// unhide contact
		db_unset(hContact, "CList", "Hidden");

		db_unset(hContact, pluginName, "MathAnswer");

		// mark contact as Answered
		db_set_b(hContact, pluginName, "Answered", 1);

		//add contact permanently
		if(gbAddPermanent) //do not use this )
			db_unset(hContact, "CList", "NotOnList");

		// send congratulation
		if(msg)
		{
			tstring prot=DBGetContactSettingStringPAN(NULL,dbei->szModule,"AM_BaseProto", _T(""));
			// for notICQ protocols or disable auto auth. reqwest
			if((Stricmp(_T("ICQ"),prot.c_str()))||(!gbAutoReqAuth))
			{
				char * buf=mir_utf8encodeW(variables_parse(gbCongratulation, hContact).c_str());
				CallContactService(hContact, PSS_MESSAGE, PREF_UTF, (LPARAM)buf);
				mir_free(buf);
			};
			// Note: For ANSI can be not work
			if(!Stricmp(_T("ICQ"),prot.c_str())){
				// grand auth.
				if(gbAutoAuth)
						CallProtoService(dbei->szModule, "/GrantAuth", w, 0);
					// add contact to server list and local group
				if(gbAutoAddToServerList)
				{
					db_set_ws(hContact, "CList", "Group", gbAutoAuthGroup.c_str());
					CallProtoService(dbei->szModule, "/AddServerContact", w, 0);
					db_unset(hContact, "CList", "NotOnList");
				};
				// auto auth. reqwest with send congratulation
				if(gbAutoReqAuth)
					CallContactService(hContact,PSS_AUTHREQUESTW,0, (LPARAM)variables_parse(gbCongratulation, hContact).c_str());
			}
		}
		return 0;
	}
	// URL contains check
	msg=(msg&&gbIgnoreURL)?(!IsUrlContains((TCHAR *) message.c_str())):msg;
	// if message message does not contain infintite talk protection prefix
	// and question count for this contact is less then maximum
	if(msg)
	{
		if((!gbInfTalkProtection || tstring::npos==message.find(_T("StopSpam automatic message:\r\n")))
			&& (!gbMaxQuestCount || db_get_dw(hContact, pluginName, "QuestionCount", 0) < gbMaxQuestCount))
		{
			// send question
			tstring q;
			if(gbInfTalkProtection)
				q += _T("StopSpam automatic message:\r\n");
			if(gbMathExpression)
			{ //parse math expression in question
				tstring tmp_question = gbQuestion;
				std::list<int> args;
				std::list<TCHAR> actions;
				tstring::size_type p1 = gbQuestion.find(_T("X")), p2 = 0;
				const tstring expr_chars = _T("X+-/*"), expr_acts = _T("+-/*");
				while(p1 < gbQuestion.length() && p1 != tstring::npos && expr_chars.find(gbQuestion[p1]) != tstring::npos)
				{
					std::string arg;
					p2 = p1;
					for(p1 = gbQuestion.find(_T("X"), p1); (p1 < gbQuestion.length()) && (gbQuestion[p1] == L'X'); ++p1)
						arg += get_random_num(1);

					tmp_question.replace(p2, arg.size(), toUTF16(arg));
					args.push_back(atoi(arg.c_str()));

					if((p1 < gbQuestion.length()) && (p1 != tstring::npos) && (expr_acts.find(gbQuestion[p1]) != tstring::npos))
						actions.push_back(gbQuestion[p1]);
					++p1;
				}
				int math_answer = 0;
				math_answer = args.front();
				args.pop_front();
				while(!args.empty())
				{
					if(!actions.empty())
					{
						switch(actions.front())
						{
						case _T('+'):
							{
								math_answer += args.front();
								args.pop_front();
							}
							break;
						case _T('-'):
							{
								math_answer -= args.front();
								args.pop_front();
							}
							break;
						case _T('/'):
							{
								math_answer /= args.front();
								args.pop_front();
							}
							break;
						case _T('*'):
							{
								math_answer *= args.front();
								args.pop_front();
							}
							break;
						}
						actions.pop_front();
					}
					else
						break;
				}
				db_set_dw(hContact, pluginName, "MathAnswer", math_answer);
				q += variables_parse(tmp_question, hContact);
			}
			else
				q += variables_parse(gbQuestion, hContact);

			char * buf=mir_utf8encodeW(q.c_str());
			CallContactService(hContact, PSS_MESSAGE, PREF_UTF, (LPARAM)buf);
			mir_free(buf);

			// increment question count
			DWORD questCount = db_get_dw(hContact, pluginName, "QuestionCount", 0);
			db_set_dw(hContact, pluginName, "QuestionCount", questCount + 1);
		}
		else
		{
/*			if (gbDosServiceExist)
			{
				if(gbDosServiceIntegration)
				{
					int i;
					i = rand()%255*13;
					CallService(MS_DOS_SERVICE, hContact, (LPARAM)i);
				}
			} */
			if(gbIgnoreContacts)
			{
				db_set_dw(hContact, "Ignore", "Mask1", 0x0000007F);
			}
		}
	}
	if(gbHideContacts)
		db_set_b(hContact, "CList", "Hidden", 1);
	if(gbSpecialGroup)
		db_set_ws(hContact, "CList", "Group", gbSpammersGroup.c_str());
	db_set_b(hContact, "CList", "NotOnList", 1);

	// save first message from contact
	if (db_get_dw(hContact, pluginName, "QuestionCount", 0)<2){
		dbei->flags |= DBEF_READ;
		db_event_add(hContact, dbei);
	};
	// reject processing of the event
	LogSpamToFile(hContact, message);
	return 1;
}

MIRANDA_HOOK_EVENT(ME_DB_CONTACT_SETTINGCHANGED, w, l)
{
	MCONTACT hContact = (MCONTACT)w;
	DBCONTACTWRITESETTING * cws = (DBCONTACTWRITESETTING*)l;

	// if CList/NotOnList is being deleted then remove answeredSetting
	if(strcmp(cws->szModule, "CList"))
		return 0;
	if(strcmp(cws->szSetting, "NotOnList"))
		return 0;
	if(!cws->value.type)
	{
		db_unset(hContact, pluginName, "Answered");
		db_unset(hContact, pluginName, "QuestionCount");
	}

	return 0;
}
