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

#include "stdafx.h"

int OnDbEventAdded(WPARAM hContact, LPARAM hDbEvent)
{
	DB::EventInfo dbei;
	dbei.cbBlob = -1;
	if (db_event_get(hDbEvent, &dbei))
		return 0;

	// if event is in protocol that is not despammed
	if (!ProtoInList(dbei.szModule))
		return 0;

	// event is an auth request
	if (gbHandleAuthReq) {
		if (!(dbei.flags & DBEF_SENT) && !(dbei.flags & DBEF_READ) && dbei.eventType == EVENTTYPE_AUTHREQUEST) {
			MCONTACT hcntct = DbGetAuthEventContact(&dbei);

			// if request is from unknown or not marked Answered contact
			int a = !Contact::OnList(hcntct);
			int b = !g_plugin.getByte(hcntct, "Answered");

			if (a && b) {
				// ...send message

				if (gbHideContacts)
					Contact::Hide(hcntct);
				if (gbSpecialGroup)
					Clist_SetGroup(hcntct, gbSpammersGroup.c_str());
				uint8_t msg = 1;
				if (gbIgnoreURL) {
					wchar_t* EventText = ReqGetText(&dbei); //else return NULL
					msg = !IsUrlContains(EventText);
					mir_free(EventText);
				}
				if (gbInvisDisable) {
					if (Proto_GetStatus(dbei.szModule) == ID_STATUS_INVISIBLE)
						msg = 0;
					else if (db_get_w(hContact, dbei.szModule, "ApparentMode", 0) == ID_STATUS_OFFLINE)
						msg = 0; //is it useful ?
				}
				if (msg) {
					ptrA buff(mir_utf8encodeW(variables_parse(gbAuthRepl, hcntct).c_str()));
					ProtoChainSend(hcntct, PSS_MESSAGE, 0, (LPARAM)buff);
				}
				return 1;
			}
		}
	}
	return 0;
}

int OnDbEventFilterAdd(WPARAM hContact, LPARAM l)
{
	if (!l) // fix potential DEP crash
		return 0;

	// if event is in protocol that is not despammed
	DBEVENTINFO * dbei = (DBEVENTINFO*)l;
	if (!ProtoInList(dbei->szModule))
		return 0; // ...let the event go its way

	// do not check excluded contact
	if (g_plugin.getByte(hContact, "Answered"))
		return 0;

	if (g_plugin.getByte(hContact, "Excluded")) {
		if (Contact::OnList(hContact))
			g_plugin.delSetting(hContact, "Excluded");
		return 0;
	}

	// we want block not only messages, i seen many types other eventtype flood
	if (dbei->flags & DBEF_READ)
		return 0; // ...let the event go its way
	
	// mark contact which we trying to contact for exclude from check
	if ((dbei->flags & DBEF_SENT) && !Contact::OnList(hContact)
		&& (!gbMaxQuestCount || g_plugin.getDword(hContact, "QuestionCount") < gbMaxQuestCount) && gbExclude) {
		g_plugin.setByte(hContact, "Excluded", 1);
		return 0;
	}
	
	// if message is from known or marked Answered contact
	if (Contact::OnList(hContact))
		return 0; // ...let the event go its way
	
	// if message is corrupted or empty it cannot be an answer.
	if (!dbei->cbBlob || !dbei->pBlob)
		// reject processing of the event
		return 1;

	wstring message;
	if (dbei->flags & DBEF_UTF)
		message = ptrW(mir_utf8decodeW((char*)dbei->pBlob));
	else
		message = _A2T((char*)(dbei->pBlob));

	// if message contains right answer...

	boost::algorithm::erase_all(message, "\r");
	boost::algorithm::erase_all(message, "\n");

	bool bSendMsg = true;
	if (gbInvisDisable) {
		if (Proto_GetStatus(dbei->szModule) == ID_STATUS_INVISIBLE)
			bSendMsg = false;
		else if (db_get_w(hContact, dbei->szModule, "ApparentMode", 0) == ID_STATUS_OFFLINE)
			bSendMsg = false; //is it useful ?
	}
	bool answered = false;
	if (gbMathExpression) {
		if (boost::algorithm::all(message, boost::is_digit())) {
			int num = _wtoi(message.c_str());
			int math_answer = g_plugin.getDword(hContact, "MathAnswer");
			if (num && math_answer)
				answered = (num == math_answer);
		}
	}
	else if (!gbRegexMatch)
		answered = gbCaseInsensitive ? (!Stricmp(message.c_str(), (variables_parse(gbAnswer, hContact).c_str()))) : (!mir_wstrcmp(message.c_str(), (variables_parse(gbAnswer, hContact).c_str())));
	else {
		if (gbCaseInsensitive) {
			std::string check(toUTF8(variables_parse(gbAnswer, hContact))), msg(toUTF8(message));
			boost::algorithm::to_upper(check);
			boost::algorithm::to_upper(msg);
			boost::regex expr(check);
			answered = boost::regex_search(msg.begin(), msg.end(), expr);
		}
		else {
			std::string check(toUTF8(variables_parse(gbAnswer, hContact))), msg(toUTF8(message));
			boost::regex expr(check);
			answered = boost::regex_search(msg.begin(), msg.end(), expr);
		}
	}
	
	if (answered) {
		// unhide contact
		Contact::Hide(hContact, false);

		g_plugin.delSetting(hContact, "MathAnswer");

		// mark contact as Answered
		g_plugin.setByte(hContact, "Answered", 1);

		//add contact permanently
		if (gbAddPermanent) //do not use this )
			Contact::PutOnList(hContact);

		// send congratulation
		if (bSendMsg) {
			wstring prot = DBGetContactSettingStringPAN(NULL, dbei->szModule, "AM_BaseProto", L"");
			// for notICQ protocols or disable auto auth. request
			if ((Stricmp(L"ICQ", prot.c_str())) || (!gbAutoReqAuth)) {
				char * buf = mir_utf8encodeW(variables_parse(gbCongratulation, hContact).c_str());
				ProtoChainSend(hContact, PSS_MESSAGE, 0, (LPARAM)buf);
				mir_free(buf);
			}
			// Note: For ANSI can be not work
			if (!Stricmp(L"ICQ", prot.c_str())) {
				// grand auth.
				if (gbAutoAuth)
					CallProtoService(dbei->szModule, "/GrantAuth", hContact, 0);
				// add contact to server list and local group
				if (gbAutoAddToServerList) {
					Clist_SetGroup(hContact, gbAutoAuthGroup.c_str());
					Contact::PutOnList(hContact);
				}
				
				// auto auth. request with send congratulation
				if (gbAutoReqAuth)
					ProtoChainSend(hContact, PSS_AUTHREQUEST, 0, (LPARAM)variables_parse(gbCongratulation, hContact).c_str());
			}
		}
		return 0;
	}
	// URL contains check
	bSendMsg = (bSendMsg && gbIgnoreURL) ? (!IsUrlContains((wchar_t *)message.c_str())) : bSendMsg;
	// if message message does not contain infintite talk protection prefix
	// and question count for this contact is less then maximum
	if (bSendMsg) {
		if ((!gbInfTalkProtection || wstring::npos == message.find(L"StopSpam automatic message:\r\n"))
			&& (!gbMaxQuestCount || g_plugin.getDword(hContact, "QuestionCount") < gbMaxQuestCount)) {
			// send question
			wstring q;
			if (gbInfTalkProtection)
				q += L"StopSpam automatic message:\r\n";
			
			if (gbMathExpression) { //parse math expression in question
				wstring tmp_question = gbQuestion;
				std::list<int> args;
				std::list<wchar_t> actions;
				wstring::size_type p1 = gbQuestion.find(L"X"), p2 = 0;
				const wstring expr_chars = L"X+-/*", expr_acts = L"+-/*";
				while (p1 < gbQuestion.length() && p1 != wstring::npos && expr_chars.find(gbQuestion[p1]) != wstring::npos) {
					std::string arg;
					p2 = p1;
					for (p1 = gbQuestion.find(L"X", p1); (p1 < gbQuestion.length()) && (gbQuestion[p1] == L'X'); ++p1)
						arg += get_random_num(1);

					tmp_question.replace(p2, arg.size(), toUTF16(arg));
					args.push_back(atoi(arg.c_str()));

					if ((p1 < gbQuestion.length()) && (p1 != wstring::npos) && (expr_acts.find(gbQuestion[p1]) != wstring::npos))
						actions.push_back(gbQuestion[p1]);
					++p1;
				}
				
				int math_answer = 0;
				math_answer = args.front();
				args.pop_front();
				while (!args.empty()) {
					if (actions.empty())
						break;

					switch (actions.front()) {
					case '+':
						math_answer += args.front();
						args.pop_front();
						break;
					case '-':
						math_answer -= args.front();
						args.pop_front();
						break;
					case '/':
						math_answer /= args.front();
						args.pop_front();
						break;
					case '*':
						math_answer *= args.front();
						args.pop_front();
						break;
					}
					actions.pop_front();
				}
				g_plugin.setDword(hContact, "MathAnswer", math_answer);
				q += variables_parse(tmp_question, hContact);
			}
			else q += variables_parse(gbQuestion, hContact);

			ProtoChainSend(hContact, PSS_MESSAGE, 0, ptrA(mir_utf8encodeW(q.c_str())));

			// increment question count
			uint32_t questCount = g_plugin.getDword(hContact, "QuestionCount");
			g_plugin.setDword(hContact, "QuestionCount", questCount + 1);
		}
		else {
			if (gbIgnoreContacts)
				db_set_dw(hContact, "Ignore", "Mask1", 0x0000007F);
		}
	}
	if (gbHideContacts)
		Contact::Hide(hContact);
	if (gbSpecialGroup)
		Clist_SetGroup(hContact, gbSpammersGroup.c_str());
	Contact::RemoveFromList(hContact);

	// save first message from contact
	if (g_plugin.getDword(hContact, "QuestionCount") < 2) {
		dbei->flags |= DBEF_READ;
		db_event_add(hContact, dbei);
	};
	// reject processing of the event
	LogSpamToFile(hContact, message);
	return 1;
}

int OnDbContactSettingChanged(WPARAM w, LPARAM l)
{
	MCONTACT hContact = (MCONTACT)w;
	DBCONTACTWRITESETTING * cws = (DBCONTACTWRITESETTING*)l;

	// if CList/NotOnList is being deleted then remove answeredSetting
	if (strcmp(cws->szModule, "CList"))
		return 0;
	if (strcmp(cws->szSetting, "NotOnList"))
		return 0;
	if (!cws->value.type) {
		g_plugin.delSetting(hContact, "Answered");
		g_plugin.delSetting(hContact, "QuestionCount");
	}

	return 0;
}
