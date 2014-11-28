//beinhaltet funktionen zum setzen des status wenn man ingame ist

#include "stdafx.h"
#include "baseProtocol.h"
#include "m_variables.h"
#include "Xfire_gamelist.h"

extern Xfire_gamelist xgamelist;


#include <vector>

using namespace std;

#include "xdebug.h"

//vector beinhaltet statusmessages
vector<string> *olstatusmsg = NULL;
vector<string> *protoname = NULL;
vector<unsigned int> *olstatus = NULL;
vector<unsigned int> *oltostatus = NULL;
PROTOACCOUNT **temp;
int anz, statusid;
int statustype;

BOOL BackupStatusMsg() {
	DBVARIANT dbv;

	statustype = db_get_b(NULL, protocolname, "statuschgtype", 0);

	XFireLog("Backup Status Message...");

	//alten vector löschen
	if (olstatusmsg != NULL)
	{
		delete olstatusmsg;
		olstatusmsg = NULL;
	}
	if (protoname != NULL)
	{
		delete protoname;
		protoname = NULL;
	}
	if (olstatus != NULL)
	{
		delete olstatus;
		olstatus = NULL;
	}
	if (oltostatus != NULL)
	{
		delete oltostatus;
		oltostatus = NULL;
	}
	olstatusmsg = new vector < string > ;
	protoname = new vector < string > ;
	olstatus = new vector < unsigned int > ;
	oltostatus = new vector < unsigned int > ;

	//alle protokolle durchgehen und den status in den vector sichern
	CallService(MS_PROTO_ENUMACCOUNTS, (WPARAM)&anz, (LPARAM)&temp);
	for (int i = 0; i < anz; i++)
	{
		statusid = CallProtoService(temp[i]->szModuleName, PS_GETSTATUS, 0, 0);
		XFireLog("Get Status of %s ...", temp[i]->szModuleName);

		char ttemp[128] = "";
		mir_snprintf(ttemp, 128, "%s%s", temp[i]->szModuleName, PS_SETAWAYMSG);

		//xfire wird geskipped, offline prots und invs prots auch, und locked status prots auch
		if (!temp[i]->bIsEnabled || statusid == ID_STATUS_INVISIBLE || statusid == ID_STATUS_OFFLINE || !lstrcmpiA(temp[i]->szModuleName, protocolname) || !ServiceExists(ttemp) || db_get_b(NULL, temp[i]->szModuleName, "LockMainStatus", 0) == 1)
		{
			XFireLog("-> Skip %s.", temp[i]->szModuleName);

			olstatus->push_back(-1);
			olstatusmsg->push_back("");
			if (statustype) oltostatus->push_back(-1);
			protoname->push_back("");
			continue;
		}

		if (statustype)
		{
			int dummystatusid = -1;

			if (statusid != 0)
			{
				int caps = CallProtoService(temp[i]->szModuleName, PS_GETCAPS, PFLAGNUM_2, 0);
				bool dndFirst = db_get_b(NULL, protocolname, "dndfirst", 0) > 0;

				if (dndFirst ? caps&PF2_HEAVYDND : caps&PF2_LIGHTDND)
				{
					dummystatusid = dndFirst ? ID_STATUS_DND : ID_STATUS_OCCUPIED;
					XFireLog("%s supports %s.", temp[i]->szModuleName, dndFirst ? "DND" : "OCCUPIED");
				}
				else if (dndFirst ? caps&PF2_LIGHTDND : caps&PF2_HEAVYDND)
				{
					dummystatusid = dndFirst ? ID_STATUS_OCCUPIED : ID_STATUS_DND;
					XFireLog("%s supports %s.", temp[i]->szModuleName, dndFirst ? "OCCUPIED" : "DND");
				}
				else if (caps&PF2_SHORTAWAY)
				{
					dummystatusid = ID_STATUS_AWAY;
					XFireLog("%s supports AWAY.", temp[i]->szModuleName);
				}
				else
				{
					dummystatusid = statusid;
					XFireLog("%s no Away???.", temp[i]->szModuleName);
				}
			}

			oltostatus->push_back(dummystatusid);
		}

		switch (statusid)
		{
		case ID_STATUS_ONLINE:
			if (db_get(NULL, "SRAway", "OnMsg", &dbv))
				olstatusmsg->push_back(Translate("Yep, I'm here."));
			break;
		case ID_STATUS_AWAY:
			if (db_get(NULL, "SRAway", "AwayMsg", &dbv))
				olstatusmsg->push_back(Translate("I've been away since %time%."));
			break;
		case ID_STATUS_NA:
			if (db_get(NULL, "SRAway", "NaMsg", &dbv))
				olstatusmsg->push_back(Translate("Give it up, I'm not in!"));
			break;
		case ID_STATUS_OCCUPIED:
			if (db_get(NULL, "SRAway", "OccupiedMsg", &dbv))
				olstatusmsg->push_back(Translate("Not right now."));
			break;
		case ID_STATUS_DND:
			if (db_get(NULL, "SRAway", "DndMsg", &dbv))
				olstatusmsg->push_back(Translate("Give a guy some peace, would ya?"));
			break;
		case ID_STATUS_FREECHAT:
			if (db_get(NULL, "SRAway", "FreeChatMsg", &dbv))
				olstatusmsg->push_back(Translate("Well, I would talk to you if Miranda ICQ supported chat"));
			break;
		case ID_STATUS_ONTHEPHONE:
			if (db_get(NULL, "SRAway", "OtpMsg", &dbv))
				olstatusmsg->push_back(Translate("That'll be the phone."));
			break;
		case ID_STATUS_OUTTOLUNCH:
			if (db_get(NULL, "SRAway", "OtlMsg", &dbv))
				olstatusmsg->push_back(Translate("Mmm... food."));
			break;
		default:
			//nix zumachn, kenn den status nicht
			olstatus->push_back(-1);
			olstatusmsg->push_back("");
			protoname->push_back("");
			continue;
		}

		//ab in den vector
		olstatus->push_back(statusid);
		if (olstatus->size() > olstatusmsg->size())
		{
			olstatusmsg->push_back(string(dbv.pszVal));
			protoname->push_back(temp[i]->szModuleName);
			//freigeben
			db_free(&dbv);
		}
		else
			protoname->push_back(temp[i]->szModuleName);
	}

	return TRUE;
}

BOOL SetGameStatusMsg()
{
	//prüfe ob vector leer
	if (olstatusmsg == NULL)
	{
		return FALSE;
	}

	ptrA statusMsg;

	//zusetzende statusmsg erstellen
	if (ServiceExists(MS_VARS_FORMATSTRING))
	{
		ptrT statusMsgT(db_get_tsa(NULL, protocolname, "setstatusmsg"));
		//direkte funktionen verwenden
		statusMsgT = variables_parse(statusMsgT, NULL, 0);
		if (statusMsgT == NULL)
			return FALSE;

		statusMsg = _T2A(statusMsgT);
	}
	else
	{
		//alternativ zweig ohne variables
		ptrA statusMsg(db_get_sa(NULL, protocolname, "setstatusmsg"));
		if (statusMsg == NULL)
			return FALSE;

		char *statusmsg;

		//statusmsg rein
		xgamelist.setString(statusMsg, &statusmsg);

		//mit den vars ersetzen beginnen
		ptrA tmp;

		//derzeitiges spiel
		tmp = db_get_sa(NULL, protocolname, "currentgamename");
		xgamelist.strreplace("%myxfiregame%", tmp, &statusmsg);

		//derzeitiges voiceprogram
		tmp = db_get_sa(NULL, protocolname, "currentvoicename");
		xgamelist.strreplace("%myxfirevoice%", tmp, &statusmsg);

		//derzeitige voiceip
		tmp = db_get_sa(NULL, protocolname, "VServerIP");
		xgamelist.strreplace("%myxfirevoiceip%", tmp, &statusmsg);

		//derzeitige gameip
		tmp = db_get_sa(NULL, protocolname, "ServerIP");
		xgamelist.strreplace("%myxfireserverip%", tmp, &statusmsg);

		statusMsg = statusmsg;
	}

	CallService(MS_PROTO_ENUMACCOUNTS, (WPARAM)&anz, (LPARAM)&temp);
	for (int i = 0; i < anz; i++)
	{
		if (olstatus->at(i) != -1)
		{
			if (statustype)
			{
				//newawaysys
				if (ServiceExists("NewAwaySystem/SetStateA"))
				{
					XFireLog("-> SetStatusMsg of %s with NewAwaySystem/SetStateA.", protoname->at(i).c_str());

					NAS_PROTOINFO npi = { 0 };
					npi.cbSize = sizeof(NAS_PROTOINFO);
					npi.szProto = (char*)protoname->at(i).c_str();
					npi.status = oltostatus->at(i);
					npi.szMsg = mir_strdup(statusMsg);
					CallService("NewAwaySystem/SetStateA", (WPARAM)&npi, 1);
				}
				else if (ServiceExists("NewAwaySystem/SetStateW")) {
					XFireLog("-> SetStatusMsg of %s with NewAwaySystem/SetStateW.", protoname->at(i).c_str());

					NAS_PROTOINFO npi = { 0 };
					npi.cbSize = sizeof(NAS_PROTOINFO);
					npi.szProto = (char*)protoname->at(i).c_str();
					npi.status = oltostatus->at(i);
					npi.szMsg = mir_strdup(statusMsg);
					CallService("NewAwaySystem/SetStateW", (WPARAM)&npi, 1);
				}
				else
				{
					XFireLog("-> SetStatusMsg of %s with Miranda with occupied status.", protoname->at(i).c_str());

					//statusmsg für beschäftigt setzen
					CallProtoService(temp[i]->szModuleName, PS_SETAWAYMSG, oltostatus->at(i), (LPARAM)statusMsg);
					//status auf beschäftigt wechseln
					CallProtoService(temp[i]->szModuleName, PS_SETSTATUS, oltostatus->at(i), 0);
					//statusmsg für beschäftigt setzen
					if (CallProtoService(temp[i]->szModuleName, PS_GETSTATUS, 0, 0) != oltostatus->at(i))
					{
						XFireLog("Set StatusMsg again, Status was not succesfully set.");
						CallProtoService(temp[i]->szModuleName, PS_SETAWAYMSG, oltostatus->at(i), (LPARAM)statusMsg);
					}
				}
			}
			else
			{
				XFireLog("-> SetStatusMsg of %s.", protoname->at(i).c_str());

				CallProtoService(temp[i]->szModuleName, PS_SETAWAYMSG, olstatus->at(i), (LPARAM)statusMsg);
			}
		}
	}

	return TRUE;
}

BOOL SetOldStatusMsg()
{
	//prüfe ob vector leer
	if (olstatusmsg == NULL)
		return FALSE;

	CallService(MS_PROTO_ENUMACCOUNTS, (WPARAM)&anz, (LPARAM)&temp);
	for (int i = 0; i < anz; i++)
	{
		if (olstatus->at(i) != -1)
		{
			if (statustype)
			{
				//alten status setzen
				CallProtoService(temp[i]->szModuleName, PS_SETSTATUS, olstatus->at(i), 0);
				//status wurde nicht gewechselt, dann statusmsg nachträglich setzen
				if (CallProtoService(temp[i]->szModuleName, PS_GETSTATUS, 0, 0) != olstatus->at(i))
					CallProtoService(temp[i]->szModuleName, PS_SETAWAYMSG, olstatus->at(i), (LPARAM)olstatusmsg->at(i).c_str());
			}
			else
			{
				CallProtoService(temp[i]->szModuleName, PS_SETSTATUS, olstatus->at(i), 0);
				CallProtoService(temp[i]->szModuleName, PS_SETAWAYMSG, olstatus->at(i), (LPARAM)olstatusmsg->at(i).c_str());
			}
		}
	}

	//alten vector löschen
	if (protoname != NULL)
	{
		delete protoname;
		protoname = NULL;
	}
	if (olstatusmsg != NULL)
	{
		delete olstatusmsg;
		olstatusmsg = NULL;
	}
	if (olstatus != NULL)
	{
		delete olstatus;
		olstatus = NULL;
	}
	if (oltostatus != NULL)
	{
		delete olstatus;
		olstatus = NULL;
	}

	return TRUE;
}