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


MIRANDA_HOOK_EVENT(ME_DB_EVENT_ADDED, wParam, lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	HANDLE hDbEvent = (HANDLE)lParam;

	DBEVENTINFO dbei = {0};
	dbei.cbSize = sizeof(dbei);
	dbei.cbBlob = CallService(MS_DB_EVENT_GETBLOBSIZE, (WPARAM)hDbEvent, 0);
	if(-1 == dbei.cbBlob) 
		return 0;
	
	dbei.pBlob = new BYTE[dbei.cbBlob];
	CallService(MS_DB_EVENT_GET, lParam, (LPARAM)&dbei);

	// if event is in protocol that is not despammed
	if(!ProtoInList(dbei.szModule)) {
		delete dbei.pBlob;
		return 0;
	}

	// event is an auth request
	if(gbHandleAuthReq)
	{
		if(!(dbei.flags & DBEF_SENT) && !(dbei.flags & DBEF_READ) && dbei.eventType == EVENTTYPE_AUTHREQUEST)
		{
			HANDLE hcntct;
			hcntct=*((PHANDLE)(dbei.pBlob+sizeof(DWORD)));

			// if request is from unknown or not marked Answered contact
			int a = DBGetContactSettingByte(hcntct, "CList", "NotOnList", 0);
			int b = !DBGetContactSettingByte(hcntct, pluginName, "Answered", 0);
			
			if(a && b)// 
			{
				// ...send message

				if(gbHideContacts)
					DBWriteContactSettingByte(hcntct, "CList", "Hidden", 1);
				if(gbSpecialGroup)
					DBWriteContactSettingTString(hcntct, "CList", "Group", gbSpammersGroup.c_str());
				BYTE msg = 1;
				if(gbIgnoreURL){
					TCHAR* EventText = ReqGetText(&dbei); //else return NULL
					msg=!IsUrlContains(EventText);
					mir_free(EventText);
				};
				if(gbInvisDisable)
				{
					if(CallProtoService(dbei.szModule, PS_GETSTATUS, 0, 0) == ID_STATUS_INVISIBLE)
						msg = 0;
					else if(DBGetContactSettingWord(hContact,dbei.szModule,"ApparentMode",0) == ID_STATUS_OFFLINE)
						msg = 0; //is it useful ? 
				}
				if(msg)
				{
					char * buff=mir_utf8encodeW(variables_parse(gbAuthRepl, hcntct).c_str());
					CallContactService(hcntct, PSS_MESSAGE, PREF_UTF, (LPARAM) buff);
					mir_free(buff);
				};
				delete dbei.pBlob;
				return 1;
			}
		}
	}
	delete dbei.pBlob;
	return 0;
}

MIRANDA_HOOK_EVENT(ME_DB_EVENT_FILTER_ADD, w, l)
{
	HANDLE hContact = (HANDLE)w;
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

	if(DBGetContactSettingByte(hContact, pluginName, "Answered", 0))
		return 0;
	if(DBGetContactSettingByte(hContact, pluginName, "Excluded", 0))
	{
		if(!DBGetContactSettingByte(hContact, "CList", "NotOnList", 0))
			DBDeleteContactSetting(hContact, pluginName, "Excluded");
		return 0;
	}
	//we want block not only messages, i seen many types other eventtype flood
	if(dbei->flags & DBEF_READ)
		// ...let the event go its way
		return 0;
	//mark contact which we trying to contact for exclude from check
	if((dbei->flags & DBEF_SENT) && DBGetContactSettingByte(hContact, "CList", "NotOnList", 0)
		&& (!gbMaxQuestCount || DBGetContactSettingDword(hContact, pluginName, "QuestionCount", 0) < gbMaxQuestCount) && gbExclude)
	{
		DBWriteContactSettingByte(hContact, pluginName, "Excluded", 1);
		return 0;
	}
	// if message is from known or marked Answered contact
	if(!DBGetContactSettingByte(hContact, "CList", "NotOnList", 0))
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
	
	BYTE msg = 1;
	if(gbInvisDisable)
	{
		if(CallProtoService(dbei->szModule, PS_GETSTATUS, 0, 0) == ID_STATUS_INVISIBLE)
			msg = 0;
		else if(DBGetContactSettingWord(hContact,dbei->szModule,"ApparentMode",0) == ID_STATUS_OFFLINE)
			msg = 0; //is it useful ?
	}
	bool answered = false;
	if(gbMathExpression)
	{
		if(boost::algorithm::all(message, boost::is_digit()))
		{
			int msg = _ttoi(message.c_str());
			int math_answer = DBGetContactSettingDword(hContact, pluginName, "MathAnswer", 0);
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
		DBDeleteContactSetting(hContact, "CList", "Hidden");

		DBDeleteContactSetting(hContact, pluginName, "MathAnswer");

		// mark contact as Answered
		DBWriteContactSettingByte(hContact, pluginName, "Answered", 1);

		//add contact permanently
		if(gbAddPermanent) //do not use this )
			DBDeleteContactSetting(hContact, "CList", "NotOnList");

		// send congratulation
		if(msg)
		{
			tstring prot=DBGetContactSettingStringPAN(NULL,dbei->szModule,"AM_BaseProto", _T(""));
			// for notICQ protocols or disable auto auth. reqwest
			if((Stricmp(_T("ICQ"),prot.c_str()))||(!gbAutoReqAuth))
			{
#ifdef _UNICODE
				char * buf=mir_utf8encodeW(variables_parse(gbCongratulation, hContact).c_str());
				CallContactService(hContact, PSS_MESSAGE, PREF_UTF, (LPARAM)buf);
				mir_free(buf);
#else
				CallContactService(hContact, PSS_MESSAGE, 0, (LPARAM)GetCongratulation().c_str());
#endif
			};
			// Note: For ANSI can be not work
			if(!Stricmp(_T("ICQ"),prot.c_str())){
				// grand auth.
				if(gbAutoAuth)
						CallProtoService(dbei->szModule, "/GrantAuth", w, 0);
					// add contact to server list and local group
				if(gbAutoAddToServerList)
				{
					DBWriteContactSettingTString(hContact, "CList", "Group", gbAutoAuthGroup.c_str());
					CallProtoService(dbei->szModule, "/AddServerContact", w, 0);
					DBDeleteContactSetting(hContact, "CList", "NotOnList");
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
			&& (!gbMaxQuestCount || DBGetContactSettingDword(hContact, pluginName, "QuestionCount", 0) < gbMaxQuestCount))
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
#ifdef UNICODE
					for(p1 = gbQuestion.find(_T("X"), p1); (p1 < gbQuestion.length()) && (gbQuestion[p1] == L'X'); ++p1)
#else
					for(p1 = gbQuestion.find(_T("X"), p1); gbQuestion[p1] == 'X'; ++p1)
#endif
						arg += get_random_num(1);
#ifdef UNICODE
					tmp_question.replace(p2, arg.size(), toUTF16(arg));
#else
					tmp_question.replace(p2, arg.size(), arg);
#endif
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
				DBWriteContactSettingDword(hContact, pluginName, "MathAnswer", math_answer);
				q += variables_parse(tmp_question, hContact);
			}
			else
				q += variables_parse(gbQuestion, hContact);

#ifdef _UNICODE
			char * buf=mir_utf8encodeW(q.c_str());
			CallContactService(hContact, PSS_MESSAGE, PREF_UTF, (LPARAM)buf);
			mir_free(buf);
#else
			CallContactService(hContact, PSS_MESSAGE, 0, (LPARAM)q.c_str());
#endif

			// increment question count
			DWORD questCount = DBGetContactSettingDword(hContact, pluginName, "QuestionCount", 0);
			DBWriteContactSettingDword(hContact, pluginName, "QuestionCount", questCount + 1);
		}
		else
		{
/*			if (gbDosServiceExist)
			{
				if(gbDosServiceIntegration)
				{
					int i;
					i = rand()%255*13;
					CallService(MS_DOS_SERVICE, (WPARAM)hContact, (LPARAM)i);
				}
			} */
			if(gbIgnoreContacts)
			{
				DBWriteContactSettingDword(hContact, "Ignore", "Mask1", 0x0000007F);
			}
		}
	}
	if(gbHideContacts)
		DBWriteContactSettingByte(hContact, "CList", "Hidden", 1);
	if(gbSpecialGroup)
		DBWriteContactSettingTString(hContact, "CList", "Group", gbSpammersGroup.c_str());
	DBWriteContactSettingByte(hContact, "CList", "NotOnList", 1);

	// save first message from contact
	if (DBGetContactSettingDword(hContact, pluginName, "QuestionCount", 0)<2){
		dbei->flags |= DBEF_READ;
		CallService(MS_DB_EVENT_ADD, (WPARAM)hContact, (LPARAM)dbei);
	};
	// reject processing of the event
	LogSpamToFile(hContact, message);
	return 1;
}


MIRANDA_HOOK_EVENT(ME_DB_CONTACT_SETTINGCHANGED, w, l)
{
	HANDLE hContact = (HANDLE)w;
	DBCONTACTWRITESETTING * cws = (DBCONTACTWRITESETTING*)l;

	// if CList/NotOnList is being deleted then remove answeredSetting
	if(strcmp(cws->szModule, "CList"))
		return 0;
	if(strcmp(cws->szSetting, "NotOnList"))
		return 0;
	if(!cws->value.type)
	{
		DBDeleteContactSetting(hContact, pluginName, "Answered");
		DBDeleteContactSetting(hContact, pluginName, "QuestionCount");
	}

	return 0;
}



