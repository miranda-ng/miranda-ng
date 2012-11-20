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
vector<string> *olstatusmsg=NULL;
vector<string> *protoname=NULL;
vector<unsigned int> *olstatus=NULL;
vector<unsigned int> *oltostatus=NULL;
PROTOACCOUNT **temp;
int anz,statusid;
int statustype;

BOOL BackupStatusMsg() {
	DBVARIANT dbv;

	statustype=DBGetContactSettingByte(NULL,protocolname,"statuschgtype",0);

	XFireLog("Backup Status Message ...");

	//alten vector löschen
	if(olstatusmsg!=NULL)
	{
		delete olstatusmsg;
		olstatusmsg=NULL;
	}
	if(protoname!=NULL)
	{
		delete protoname;
		protoname=NULL;
	}
	if(olstatus!=NULL)
	{
		delete olstatus;
		olstatus=NULL;
	}
	if(oltostatus!=NULL)
	{
		delete oltostatus;
		oltostatus=NULL;
	}
	olstatusmsg=new vector<string>;
	protoname=new vector<string>;
	olstatus=new vector<unsigned int>;
	oltostatus=new vector<unsigned int>;

	//alle protokolle durchgehen und den status in den vector sichern
	CallService(MS_PROTO_ENUMACCOUNTS,(WPARAM)&anz,(LPARAM)&temp);
	for(int i=0;i<anz;i++)
	{
		statusid=CallProtoService(temp[i]->szModuleName,PS_GETSTATUS,0,0);
		XFireLog("Get Status of %s ...",temp[i]->szModuleName);

		char ttemp[128]="";
		sprintf_s(ttemp,128,"%s%s",temp[i]->szModuleName,PS_SETAWAYMSG);
		
		//xfire wird geskipped, offline prots und invs prots auch, und locked status prots auch
		if(!temp[i]->bIsEnabled||statusid==ID_STATUS_INVISIBLE||statusid==ID_STATUS_OFFLINE||!lstrcmpiA( temp[i]->szModuleName, protocolname )||!ServiceExists(ttemp)||DBGetContactSettingByte(NULL,temp[i]->szModuleName,"LockMainStatus",0)==1)
		{
			XFireLog("-> Skip %s.",temp[i]->szModuleName);

			olstatus->push_back(-1);
			olstatusmsg->push_back("");
			if(statustype) oltostatus->push_back(-1);
			protoname->push_back("");
			continue;
		}

		int dummystatusid=statusid;

		if(statustype) 
		{
			if(statusid!=0)
			{
				int caps=CallProtoService(temp[i]->szModuleName,PS_GETCAPS,PFLAGNUM_2,0);

				//normale statusreihenfolge
				if(DBGetContactSettingByte(NULL,protocolname,"dndfirst",0)==0)
				{
					if(caps&PF2_LIGHTDND)
					{
						oltostatus->push_back(ID_STATUS_OCCUPIED);
						dummystatusid=ID_STATUS_OCCUPIED;
						XFireLog("%s supports OCCUPIED.",temp[i]->szModuleName);
					}
					else if(caps&PF2_HEAVYDND)
					{
						oltostatus->push_back(ID_STATUS_DND);
						dummystatusid=ID_STATUS_DND;
						XFireLog("%s supports DND.",temp[i]->szModuleName);
					}
					else if(caps&PF2_SHORTAWAY)
					{
						oltostatus->push_back(ID_STATUS_AWAY);
						dummystatusid=ID_STATUS_AWAY;
						XFireLog("%s supports AWAY.",temp[i]->szModuleName);
					}
					else
					{
						XFireLog("%s no Away???.",temp[i]->szModuleName);
						oltostatus->push_back(statusid);
						dummystatusid=statusid;
					}
				}
				else //dnd bevorzugt
				{
					if(caps&PF2_HEAVYDND)
					{
						oltostatus->push_back(ID_STATUS_DND);
						dummystatusid=ID_STATUS_DND;
						XFireLog("%s supports DND.",temp[i]->szModuleName);
					}
					else if(caps&PF2_LIGHTDND)
					{
						oltostatus->push_back(ID_STATUS_OCCUPIED);
						dummystatusid=ID_STATUS_OCCUPIED;
						XFireLog("%s supports OCCUPIED.",temp[i]->szModuleName);
					}
					else if(caps&PF2_SHORTAWAY)
					{
						oltostatus->push_back(ID_STATUS_AWAY);
						dummystatusid=ID_STATUS_AWAY;
						XFireLog("%s supports AWAY.",temp[i]->szModuleName);
					}
					else
					{
						XFireLog("%s no Away???.",temp[i]->szModuleName);
						oltostatus->push_back(statusid);
						dummystatusid=statusid;
					}
				}
			}
			else
				oltostatus->push_back(-1);
		}

		switch(statusid)
		{
		case ID_STATUS_ONLINE:
			if(DBGetContactSetting(NULL,"SRAway","OnMsg",&dbv))
				olstatusmsg->push_back(Translate("Yep, I'm here."));
			break;
		case ID_STATUS_AWAY:
			if(DBGetContactSetting(NULL,"SRAway","AwayMsg",&dbv))
				olstatusmsg->push_back(Translate("I've been away since %time%."));
			break;
		case ID_STATUS_NA:
			if(DBGetContactSetting(NULL,"SRAway","NaMsg",&dbv))
				olstatusmsg->push_back(Translate("Give it up, I'm not in!"));
			break;
		case ID_STATUS_OCCUPIED:
			if(DBGetContactSetting(NULL,"SRAway","OccupiedMsg",&dbv))
				olstatusmsg->push_back(Translate("Not right now."));
			break;
		case ID_STATUS_DND:
			if(DBGetContactSetting(NULL,"SRAway","DndMsg",&dbv))
				olstatusmsg->push_back(Translate("Give a guy some peace, would ya?"));
			break;
		case ID_STATUS_FREECHAT:
			if(DBGetContactSetting(NULL,"SRAway","FreeChatMsg",&dbv))
				olstatusmsg->push_back(Translate("Well, I would talk to you if Miranda ICQ supported chat"));
			break;
		case ID_STATUS_ONTHEPHONE:
			if(DBGetContactSetting(NULL,"SRAway","OtpMsg",&dbv))
				olstatusmsg->push_back(Translate("That'll be the phone."));
			break;
		case ID_STATUS_OUTTOLUNCH:
			if(DBGetContactSetting(NULL,"SRAway","OtlMsg",&dbv))
				olstatusmsg->push_back(Translate("Mmm...food."));
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
		if(olstatus->size()>olstatusmsg->size())
		{
			olstatusmsg->push_back(string(dbv.pszVal));
			protoname->push_back(temp[i]->szModuleName);
			//freigeben
			DBFreeVariant(&dbv);
		}
		else
			protoname->push_back(temp[i]->szModuleName);
	}

	return TRUE;
}

BOOL SetGameStatusMsg()
{
	char* statusmsg=NULL;

	//prüfe ob vector leer
	if(olstatusmsg==NULL)
		return FALSE;

	//zusetzende statusmsg erstellen
	if (ServiceExists(MS_VARS_FORMATSTRING))
	{
		DBVARIANT dbv;
		if(!DBGetContactSetting(NULL,protocolname,"setstatusmsg",&dbv)) {
			//direkte funktionen verwenden
			statusmsg = variables_parse(dbv.pszVal,NULL,0);
			if (statusmsg == NULL)
			{
				DBFreeVariant(&dbv);
				return FALSE;
			}

			DBFreeVariant(&dbv);
		}
	}
	else
	{
		//alternativ zweig ohne variables
		DBVARIANT dbv;
		if(!DBGetContactSetting(NULL,protocolname,"setstatusmsg",&dbv)) {

			DBVARIANT dbv3;

			//statusmsg rein
			xgamelist.setString(dbv.pszVal,&statusmsg);

			//mit den vars ersetzen beginnen

			//derzeitiges spiel
			if(!DBGetContactSetting(NULL,protocolname, "currentgamename",&dbv3))
			{
				xgamelist.strreplace("%myxfiregame%",dbv3.pszVal,&statusmsg);
				DBFreeVariant(&dbv3);
			}
			else
				xgamelist.strreplace("%myxfiregame%","",&statusmsg);

			//derzeitiges voiceprogram
			if(!DBGetContactSetting(NULL,protocolname, "currentvoicename",&dbv3))
			{
				xgamelist.strreplace("%myxfirevoice%",dbv3.pszVal,&statusmsg);
				DBFreeVariant(&dbv3);
			}
			else
				xgamelist.strreplace("%myxfirevoice%","",&statusmsg);

			//derzeitige voiceip
			if(!DBGetContactSetting(NULL,protocolname, "VServerIP",&dbv3))
			{
				xgamelist.strreplace("%myxfirevoiceip%",dbv3.pszVal,&statusmsg);
				DBFreeVariant(&dbv3);
			}
			else
				xgamelist.strreplace("%myxfirevoiceip%","",&statusmsg);

			//derzeitige gameip
			if(!DBGetContactSetting(NULL,protocolname, "ServerIP",&dbv3))
			{
				xgamelist.strreplace("%myxfireserverip%",dbv3.pszVal,&statusmsg);
				DBFreeVariant(&dbv3);
			}
			else
				xgamelist.strreplace("%myxfireserverip%","",&statusmsg);

		}
	}

	CallService(MS_PROTO_ENUMACCOUNTS,(WPARAM)&anz,(LPARAM)&temp);
	for(int i=0;i<anz;i++)
	{
		if(olstatus->at(i)!=-1)
		{
			if(statustype)
			{
				//newawaysys
				if(ServiceExists("NewAwaySystem/SetStateA"))
				{
					XFireLog("-> SetStatusMsg of %s with NewAwaySystem/SetStateA.",protoname->at(i).c_str());

					NAS_PROTOINFO npi = {0};
					npi.cbSize=sizeof(NAS_PROTOINFO);
					npi.szProto=(char*)protoname->at(i).c_str();
					npi.status=oltostatus->at(i);
					npi.szMsg=mir_strdup(statusmsg);
					CallService("NewAwaySystem/SetStateA", (WPARAM)&npi, (LPARAM)1);
				}
				else if(ServiceExists("NewAwaySystem/SetStateW")) {
					XFireLog("-> SetStatusMsg of %s with NewAwaySystem/SetStateW.",protoname->at(i).c_str());

					NAS_PROTOINFO npi = {0};
					npi.cbSize=sizeof(NAS_PROTOINFO);
					npi.szProto=(char*)protoname->at(i).c_str();
					npi.status=oltostatus->at(i);
					npi.szMsg=mir_strdup(statusmsg);
					CallService("NewAwaySystem/SetStateW", (WPARAM)&npi, (LPARAM)1);
				}
				else
				{
					XFireLog("-> SetStatusMsg of %s with Miranda with occupied status.",protoname->at(i).c_str());

					//statusmsg für beschäftigt setzen
					CallProtoService(temp[i]->szModuleName,PS_SETAWAYMSG,oltostatus->at(i),(LPARAM)statusmsg);
					//status auf beschäftigt wechseln
					CallProtoService(temp[i]->szModuleName,PS_SETSTATUS,oltostatus->at(i),0);
					//statusmsg für beschäftigt setzen
					if(CallProtoService(temp[i]->szModuleName,PS_GETSTATUS,0,0)!=oltostatus->at(i))
					{
						XFireLog("Set StatusMsg again, Status was not succesfully set.");
						CallProtoService(temp[i]->szModuleName,PS_SETAWAYMSG,oltostatus->at(i),(LPARAM)statusmsg);
					}
				}
			}
			else
			{
				XFireLog("-> SetStatusMsg of %s.",protoname->at(i).c_str());

				CallProtoService(temp[i]->szModuleName,PS_SETAWAYMSG,olstatus->at(i),(LPARAM)statusmsg);
			}
		}
	}

	if (ServiceExists(MS_VARS_FORMATSTRING))
		CallService(MS_VARS_FREEMEMORY, (WPARAM)statusmsg, 0);
	else
		if(statusmsg) delete[] statusmsg;

	return TRUE;
}

BOOL SetOldStatusMsg()
{
	//prüfe ob vector leer
	if(olstatusmsg==NULL)
		return FALSE;

	CallService(MS_PROTO_ENUMACCOUNTS,(WPARAM)&anz,(LPARAM)&temp);
	for(int i=0;i<anz;i++)
	{
		if(olstatus->at(i)!=-1)
		{
			if(statustype)
			{
				//alten status setzen
				CallProtoService(temp[i]->szModuleName,PS_SETSTATUS,olstatus->at(i),0);
				//status wurde nicht gewechselt, dann statusmsg nachträglich setzen
				if(CallProtoService(temp[i]->szModuleName,PS_GETSTATUS,0,0)!=olstatus->at(i))
					CallProtoService(temp[i]->szModuleName,PS_SETAWAYMSG,olstatus->at(i),(LPARAM)olstatusmsg->at(i).c_str());
			}
			else
			{
				CallProtoService(temp[i]->szModuleName,PS_SETSTATUS,olstatus->at(i),0);
				CallProtoService(temp[i]->szModuleName,PS_SETAWAYMSG,olstatus->at(i),(LPARAM)olstatusmsg->at(i).c_str());
			}
		}
	}

	//alten vector löschen
	if(protoname!=NULL)
	{
		delete protoname;
		protoname=NULL;
	}
	if(olstatusmsg!=NULL)
	{
		delete olstatusmsg;
		olstatusmsg=NULL;
	}
	if(olstatus!=NULL)
	{
		delete olstatus;
		olstatus=NULL;
	}
	if(oltostatus!=NULL)
	{
		delete olstatus;
		olstatus=NULL;
	}

	return TRUE;
}