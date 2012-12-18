#include "skype_proto.h"
#include <m_chat.h>

bool CSkypeProto::IsChatRoom(HANDLE hContact)
{
	return ::DBGetContactSettingByte(hContact, this->m_szModuleName, "ChatRoom", 0) > 0;
}

void CSkypeProto::ChatValidateContact(HANDLE hItem, HWND hwndList)
{
	if ( !this->IsProtoContact(hItem) || this->IsChatRoom(hItem)) 
		::SendMessage(hwndList, CLM_DELETEITEM, (WPARAM)hItem, 0);
}

void CSkypeProto::ChatPrepare(HANDLE hItem, HWND hwndList)
{
	if (hItem == NULL)
		hItem = (HANDLE)::SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_ROOT, 0);

	while (hItem) 
	{
		HANDLE hItemN = (HANDLE)::SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXT, (LPARAM)hItem);

		if (IsHContactGroup(hItem))
		{
			HANDLE hItemT = (HANDLE)::SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);
			if (hItemT) this->ChatPrepare(hItemT, hwndList);
		}
		else if (IsHContactContact(hItem))
			this->ChatValidateContact(hItem, hwndList);

		hItem = hItemN;
   }
}

void CSkypeProto::FillChatList(HANDLE hItem, HWND hwndList, SEStringList &chatTargets)
{
	if (hItem == NULL)
		hItem = (HANDLE)::SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_ROOT, 0);

	while (hItem) 
	{
		if (IsHContactGroup(hItem))
		{
			HANDLE hItemT = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);
			if (hItemT) this->FillChatList(hItemT, hwndList, chatTargets);
		}
		else
		{
			int chk = SendMessage(hwndList, CLM_GETCHECKMARK, (WPARAM)hItem, 0);
			if (chk)
			{
				if (IsHContactInfo(hItem))
				{
					TCHAR buf[128] = _T("");
					SendMessage(hwndList, CLM_GETITEMTEXT, (WPARAM)hItem, (LPARAM)buf);

					if (buf[0]) chatTargets.append(mir_t2a(buf));
				}
				else 
				{
					char *sid = ::DBGetString(hItem, this->m_szModuleName, "sid");
					if (sid) chatTargets.append(sid);
				}
			}
		}
		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXT, (LPARAM)hItem);
	}
}

static const COLORREF crCols[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

void CSkypeProto::RegisterChat()
{
	GCREGISTER gcr = {0};
	gcr.cbSize = sizeof(gcr);
	gcr.dwFlags = GC_TYPNOTIF | GC_CHANMGR;// | GC_TCHAR;
	gcr.iMaxText = 0;
	gcr.nColors = 16;
	gcr.pColors = (COLORREF*)crCols;
	gcr.pszModuleDispName = ::mir_u2a(this->m_tszUserName);
	gcr.pszModule = this->m_szModuleName;
	CallServiceSync(MS_GC_REGISTER, 0, (LPARAM)&gcr);

	//YHookEvent(ME_GC_EVENT, &CYahooProto::OnGCEventHook);
	//YHookEvent(ME_GC_BUILDMENU, &CYahooProto::OnGCMenuHook);
}

void CSkypeProto::StartChat(SEStringList &chatTargets)
{
	CConversation::Ref conference;
	g_skype->CreateConference(conference);
	conference->AddConsumers(chatTargets);	

	SEString identity; 
	conference->GetPropIdentity(identity);
	char *chatID = ::mir_strdup((const char *)identity);

	GCSESSION gcw = {0};
	gcw.cbSize = sizeof(gcw);
	//gcw.dwFlags = GC_TCHAR;
	gcw.iType = GCW_CHATROOM;
	gcw.pszModule = m_szModuleName;
	gcw.pszName = "Chat";
	gcw.pszID = chatID;
	::CallServiceSync(MS_GC_NEWSESSION, 0, (LPARAM)&gcw);

	GCDEST gcd = { m_szModuleName, { NULL }, GC_EVENT_ADDGROUP };
	gcd.pszID = chatID;

	GCEVENT gce = {0};
	gce.cbSize = sizeof(GCEVENT);
	//gce.dwFlags = GC_TCHAR;
	gce.pDest = &gcd;
	gce.pszStatus = Translate("Me");
	::CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);

	gcd.iType = GC_EVENT_ADDGROUP;
	gce.pszStatus = Translate("Others");
	::CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);

	for (uint i = 0; i < chatTargets.size(); i++)
	{
		HANDLE hContact = this->GetContactBySid(chatTargets[i]);
		gcd.iType = GC_EVENT_JOIN;
		gce.pszNick = ::DBGetString(hContact, this->m_szModuleName, "Nick");
		gce.pszUID = chatTargets[i];
		::CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
	}

	gcd.iType = GC_EVENT_CONTROL;
	::CallServiceSync(MS_GC_EVENT, SESSION_INITDONE, (LPARAM)&gce);
	::CallServiceSync(MS_GC_EVENT, SESSION_ONLINE,   (LPARAM)&gce);
	::CallServiceSync(MS_GC_EVENT, WINDOW_VISIBLE,   (LPARAM)&gce);

	::mir_free(chatID);
}