/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.

Copyright (c) 2012-2014 Miranda NG Team
Copyright (c) 2007-2012 Boris Krasnovskiy.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "msn_global.h"
#include "msn_proto.h"

static const char abReqHdr[] =
	"SOAPAction: http://www.msn.com/webservices/AddressBook/%s\r\n";


ezxml_t CMsnProto::abSoapHdr(const char* service, const char* scenario, ezxml_t& tbdy, char*& httphdr)
{
	ezxml_t xmlp = ezxml_new("soap:Envelope");
	ezxml_set_attr(xmlp, "xmlns:soap", "http://schemas.xmlsoap.org/soap/envelope/");
	ezxml_set_attr(xmlp, "xmlns:xsi",  "http://www.w3.org/2001/XMLSchema-instance");
	ezxml_set_attr(xmlp, "xmlns:xsd",  "http://www.w3.org/2001/XMLSchema");
	ezxml_set_attr(xmlp, "xmlns:soapenc", "http://schemas.xmlsoap.org/soap/encoding/");

	ezxml_t hdr = ezxml_add_child(xmlp, "soap:Header", 0);
	ezxml_t apphdr = ezxml_add_child(hdr, "ABApplicationHeader", 0);
	ezxml_set_attr(apphdr, "xmlns", "http://www.msn.com/webservices/AddressBook");
	ezxml_t node = ezxml_add_child(apphdr, "ApplicationId", 0);
	ezxml_set_txt(node, msnAppID);
	node = ezxml_add_child(apphdr, "IsMigration", 0);
	ezxml_set_txt(node, abchMigrated ? "false" : "true");
	node = ezxml_add_child(apphdr, "PartnerScenario", 0);
	ezxml_set_txt(node, scenario);

	char *cacheKey = strstr(service, "Member") ? sharingCacheKey : abCacheKey;
	if (cacheKey)
	{
		node = ezxml_add_child(apphdr, "CacheKey", 0);
		ezxml_set_txt(node, cacheKey);
	}

	ezxml_t authhdr = ezxml_add_child(hdr, "ABAuthHeader", 0);
	ezxml_set_attr(authhdr, "xmlns", "http://www.msn.com/webservices/AddressBook");
	node = ezxml_add_child(authhdr, "ManagedGroupRequest", 0);
	ezxml_set_txt(node, "false");
	node = ezxml_add_child(authhdr, "TicketToken", 0);
	if (authContactToken) ezxml_set_txt(node, authContactToken);

	ezxml_t bdy = ezxml_add_child(xmlp, "soap:Body", 0);

	tbdy = ezxml_add_child(bdy, service, 0);
	ezxml_set_attr(tbdy, "xmlns", "http://www.msn.com/webservices/AddressBook");

	if (strstr(service, "Member") == NULL && strcmp(service, "ABAdd") != 0 && strcmp(service, "ABFindContactsPaged"))
	{
		ezxml_t node = ezxml_add_child(tbdy, "abId", 0);
		ezxml_set_txt(node, "00000000-0000-0000-0000-000000000000");
	}

	size_t hdrsz = strlen(service) + sizeof(abReqHdr) + 20;
	httphdr = (char*)mir_alloc(hdrsz);

	mir_snprintf(httphdr, hdrsz, abReqHdr, service);

	return xmlp;
}


ezxml_t CMsnProto::getSoapResponse(ezxml_t bdy, const char* service)
{
	char resp1[40], resp2[40];
	mir_snprintf(resp1, sizeof(resp1), "%sResponse", service);
	mir_snprintf(resp2, sizeof(resp2), "%sResult", service);

	ezxml_t res = ezxml_get(bdy, "soap:Body", 0, resp1, 0, resp2, -1);
	if (res == NULL)
		res = ezxml_get(bdy, "s:Body", 0, resp1, 0, resp2, -1);

	return res;
}

ezxml_t CMsnProto::getSoapFault(ezxml_t bdy, bool err)
{
	ezxml_t flt = ezxml_get(bdy, "soap:Body", 0, "soap:Fault", -1);
	return err ? ezxml_get(flt, "detail", 0, "errorcode", -1) : flt;
}

void CMsnProto::UpdateABHost(const char* service, const char* url)
{
	char hostname[128];
	mir_snprintf(hostname, sizeof(hostname), "ABHost-%s", service);

	if (url)
		setString(hostname, url);
	else
		delSetting(hostname);
}

void CMsnProto::UpdateABCacheKey(ezxml_t bdy,  bool isSharing)
{
	ezxml_t hdr = ezxml_get(bdy, "soap:Header", 0, "ServiceHeader", -1);
	bool changed = strcmp(ezxml_txt(ezxml_child(hdr, "CacheKeyChanged")), "true") == 0;
	if (changed)
	{
		replaceStr(isSharing ? sharingCacheKey : abCacheKey, ezxml_txt(ezxml_child(hdr, "CacheKey")));
	}
}

char* CMsnProto::GetABHost(const char* service, bool isSharing)
{
	char hostname[128];
	mir_snprintf(hostname, sizeof(hostname), "ABHost-%s", service);

	char* host = (char*)mir_alloc(256);
	if (db_get_static(NULL, m_szModuleName, hostname, host, 256))
	{
		mir_snprintf(host, 256, "https://byrdr.omega.contacts.msn.com/abservice/%s.asmx",
			isSharing ? "SharingService" : "abservice");
	}

	return host;
}

/*
ezxml_t CMsnProto::PerformSoapReq(const char *service, bool isSharing, char *szData, const char* hdrs, unsigned& status)
{
	unsigned status = 0;
	char *abUrl = NULL, *tResult = NULL;

	for (int k = 4; --k;)
	{
		mir_free(abUrl);
		abUrl = GetABHost(service, true);
		tResult = getSslResult(&abUrl, szData, hdrs, status);
		if (tResult == NULL) UpdateABHost(service, NULL);
		else break;
	}

	mir_free(reqHdr);
	free(szData);

	if (tResult != NULL)
	{
		UpdateABHost(service, abUrl);
		ezxml_t xmlm = ezxml_parse_str(tResult, strlen(tResult));
		if (!xmlm || !ezxml_child(xmlm, "soap:Body"))
		{
			mir_free(tResult);
			ezxml_free(xmlm);
			UpdateABHost("service", NULL);
			PerformSoapReq(service, isSharing, szData, hdrs, status);
		}
		else if (status == 500)
		{
			const char* szErr = ezxml_txt(getSoapFault(xmlm, true));
			if (!szErr[0])
			{
				mir_free(tResult);
				ezxml_free(xmlm);
				UpdateABHost("service", NULL);
				PerformSoapReq(service, isSharing, szData, hdrs, status);
			}
		}
	}
	mir_free(abUrl);
}
*/


bool CMsnProto::MSN_ABAdd(bool allowRecurse)
{
	char* reqHdr;
	ezxml_t tbdy, node;
	ezxml_t xmlp = abSoapHdr("ABAdd", "Timer", tbdy, reqHdr);

	ezxml_t abinf = ezxml_add_child(tbdy, "abInfo", 0);
	ezxml_add_child(abinf, "name", 0);
	node = ezxml_add_child(abinf, "ownerPuid", 0);
	ezxml_set_txt(node, "0");
	node = ezxml_add_child(abinf, "ownerEmail", 0);
	ezxml_set_txt(node, MyOptions.szEmail);
	node = ezxml_add_child(abinf, "fDefault", 0);
	ezxml_set_txt(node, "true");

	char* szData = ezxml_toxml(xmlp, true);
	ezxml_free(xmlp);

	unsigned status = 0;
	char *abUrl = NULL, *tResult = NULL;

	for (int k = 4; --k;)
	{
		mir_free(abUrl);
		abUrl = GetABHost("ABAdd", false);
		tResult = getSslResult(&abUrl, szData, reqHdr, status);
		if (tResult == NULL) UpdateABHost("ABAdd", NULL);
		else break;
	}

	mir_free(reqHdr);
	free(szData);

	if (tResult != NULL)
	{
		UpdateABHost("ABAdd", abUrl);

		ezxml_t xmlm = ezxml_parse_str(tResult, strlen(tResult));
		if (status == 500)
		{
			const char* szErr = ezxml_txt(getSoapFault(xmlm, true));
			if (strcmp(szErr, "PassportAuthFail") == 0 && allowRecurse)
			{
				MSN_GetPassportAuth();
				status = MSN_ABAdd(false) ? 200 : 500;
			}
		}
		ezxml_free(xmlm);
	}

	mir_free(tResult);
	mir_free(abUrl);

	return status == 200;
}


bool CMsnProto::MSN_SharingFindMembership(bool deltas, bool allowRecurse)
{
	char* reqHdr;
	ezxml_t tbdy;
	ezxml_t xmlp = abSoapHdr("FindMembership", "Initial", tbdy, reqHdr);

	ezxml_t svcflt = ezxml_add_child(tbdy, "serviceFilter", 0);
	ezxml_t tps = ezxml_add_child(svcflt, "Types", 0);
	ezxml_t node = ezxml_add_child(tps, "ServiceType", 0);
	ezxml_set_txt(node, "Messenger");
/*
	node = ezxml_add_child(tps, "ServiceType", 0);
	ezxml_set_txt(node, "Invitation");
	node = ezxml_add_child(tps, "ServiceType", 0);
	ezxml_set_txt(node, "SocialNetwork");
	node = ezxml_add_child(tps, "ServiceType", 0);
	ezxml_set_txt(node, "Space");
	node = ezxml_add_child(tps, "ServiceType", 0);
	ezxml_set_txt(node, "Profile");
	node = ezxml_add_child(tps, "ServiceType", 0);
	ezxml_set_txt(node, "Folder");
	node = ezxml_add_child(tps, "ServiceType", 0);
	ezxml_set_txt(node, "OfficeLiveWebNotification");
*/
	const char *szLastChange = NULL;
	if (deltas)
	{
		DBVARIANT dbv;
		if (!getString("SharingLastChange", &dbv) && dbv.pszVal[0])
		{
			szLastChange = NEWSTR_ALLOCA(dbv.pszVal);
			db_free(&dbv);
		}
		deltas &= (szLastChange != NULL);
	}

	node = ezxml_add_child(tbdy, "View", 0);
	ezxml_set_txt(node, "Full");
	node = ezxml_add_child(tbdy, "deltasOnly", 0);
	ezxml_set_txt(node, deltas ? "true" : "false");
	node = ezxml_add_child(tbdy, "lastChange", 0);
	ezxml_set_txt(node, deltas ? szLastChange : "0001-01-01T00:00:00.0000000-08:00");

	char* szData = ezxml_toxml(xmlp, true);

	ezxml_free(xmlp);

	unsigned status = 0;
	char *abUrl = NULL, *tResult = NULL;

	for (int k = 4; --k;)
	{
		mir_free(abUrl);
		abUrl = GetABHost("FindMembership", true);
		tResult = getSslResult(&abUrl, szData, reqHdr, status);
		if (tResult == NULL) UpdateABHost("FindMembership", NULL);
		else break;
	}

	mir_free(reqHdr);
	free(szData);

	if (tResult != NULL)
	{
		ezxml_t xmlm = ezxml_parse_str(tResult, strlen(tResult));
		if (status == 200)
		{
			UpdateABCacheKey(xmlm, true);
			ezxml_t body = getSoapResponse(xmlm, "FindMembership");
			ezxml_t svcs = ezxml_get(body, "Services", 0, "Service", -1);

			UpdateABHost("FindMembership", body ? abUrl : NULL);

			while (svcs != NULL)
			{
				const char* szType = ezxml_txt(ezxml_get(svcs, "Info", 0, "Handle", 0, "Type", -1));
				if (_stricmp(szType, "Messenger") == 0) break;
				svcs = ezxml_next(svcs);
			}

			const char* szLastChange = ezxml_txt(ezxml_child(svcs, "LastChange"));
			if (szLastChange[0]) setString("SharingLastChange", szLastChange);

			ezxml_t mems = ezxml_get(svcs, "Memberships", 0, "Membership", -1);

			while (mems != NULL)
			{
				const char* szRole = ezxml_txt(ezxml_child(mems, "MemberRole"));

				int lstId = 0;
				if (strcmp(szRole, "Allow") == 0)			lstId = LIST_AL;
				else if (strcmp(szRole, "Block") == 0)		lstId = LIST_BL;
				else if (strcmp(szRole, "Reverse") == 0)	lstId = LIST_RL;
				else if (strcmp(szRole, "Pending") == 0)	lstId = LIST_PL;

				ezxml_t memb = ezxml_get(mems, "Members", 0, "Member", -1);
				while (memb != NULL)
				{
					bool deleted = strcmp(ezxml_txt(ezxml_child(memb, "Deleted")), "true") == 0;
					const char* szType = ezxml_txt(ezxml_child(memb, "Type"));
					if (strcmp(szType, "Passport") == 0)
					{
						const char* szInvite = NULL;
						const char* szEmail = ezxml_txt(ezxml_child(memb, "PassportName"));
						const char* szNick = ezxml_txt(ezxml_child(memb, "DisplayName")); if (!szNick[0]) szNick = NULL;
						ezxml_t anot = ezxml_get(memb, "Annotations", 0, "Annotation", -1);
						while (anot != NULL)
						{
							if (strcmp(ezxml_txt(ezxml_child(anot, "Name")), "MSN.IM.InviteMessage") == 0)
							{
								szInvite = ezxml_txt(ezxml_child(anot, "Value"));
							}
							anot = ezxml_next(anot);
						}
						if (!deleted) Lists_Add(lstId, NETID_MSN, szEmail, NULL, szNick, szInvite); else Lists_Remove(lstId, szEmail);
					}
					else if (strcmp(szType, "Phone") == 0)
					{
						const char* szEmail = ezxml_txt(ezxml_child(memb, "PhoneNumber"));
						char email[128];
						mir_snprintf(email, sizeof(email), "tel:%s", szEmail);
						if (!deleted) Lists_Add(lstId, NETID_MOB, email); else Lists_Remove(lstId, szEmail);
					}
					else if (strcmp(szType, "Email") == 0)
					{
						const char* szInvite = NULL;
						const char* szEmail = ezxml_txt(ezxml_child(memb, "Email"));
						const char* szNick = ezxml_txt(ezxml_child(memb, "DisplayName")); if (!szNick[0]) szNick = NULL;
						int netId = strstr(szEmail, "@yahoo.com") ? NETID_YAHOO : NETID_LCS;
						ezxml_t anot = ezxml_get(memb, "Annotations", 0, "Annotation", -1);
						while (anot != NULL)
						{
							if (strcmp(ezxml_txt(ezxml_child(anot, "Name")), "MSN.IM.BuddyType") == 0)
							{
								netId = atol(ezxml_txt(ezxml_child(anot, "Value")));
							}
							else if (strcmp(ezxml_txt(ezxml_child(anot, "Name")), "MSN.IM.InviteMessage") == 0)
							{
								szInvite = ezxml_txt(ezxml_child(anot, "Value"));
							}
							anot = ezxml_next(anot);
						}

						if (!deleted) Lists_Add(lstId, netId, szEmail, NULL, szNick, szInvite);  else Lists_Remove(lstId, szEmail);
					}
					memb = ezxml_next(memb);
				}
				mems = ezxml_next(mems);
			}
		}
		else if (status == 500)
		{
			const char* szErr = ezxml_txt(getSoapFault(xmlm, true));
			if (strcmp(szErr, "ABDoesNotExist") == 0)
			{
				MSN_ABAdd();
				status = 200;
			}
			else if (strcmp(szErr, "PassportAuthFail") == 0 && allowRecurse)
			{
				MSN_GetPassportAuth();
				status = MSN_SharingFindMembership(deltas, false) ? 200 : 500;
			}
		}
		else
			UpdateABHost("FindMembership", NULL);

		ezxml_free(xmlm);
	}
	mir_free(tResult);
	mir_free(abUrl);

	return status == 200;
}

// AddMember, DeleteMember
bool CMsnProto::MSN_SharingAddDelMember(const char* szEmail, const int listId, const int netId, const char* szMethod, bool allowRecurse)
{
	const char* szRole;
	if (listId & LIST_AL) szRole = "Allow";
	else if (listId & LIST_BL) szRole = "Block";
	else if (listId & LIST_PL) szRole = "Pending";
	else if (listId & LIST_RL) szRole = "Reverse";
	else return false;

	char* reqHdr;
	ezxml_t tbdy;
	ezxml_t xmlp = abSoapHdr(szMethod, "BlockUnblock", tbdy, reqHdr);

	ezxml_t svchnd = ezxml_add_child(tbdy, "serviceHandle", 0);
	ezxml_t node = ezxml_add_child(svchnd, "Id", 0);
	ezxml_set_txt(node, "0");
	node = ezxml_add_child(svchnd, "Type", 0);
	ezxml_set_txt(node, "Messenger");
	node = ezxml_add_child(svchnd, "ForeignId", 0);
//	ezxml_set_txt(node, "");

	const char* szMemberName = "";
	const char* szTypeName = "";
	const char* szAccIdName = "";

	switch (netId)
	{
		case 1:
			szMemberName = "PassportMember";
			szTypeName = "Passport";
			szAccIdName = "PassportName";
			break;

		case 4:
			szMemberName = "PhoneMember";
			szTypeName = "Phone";
			szAccIdName = "PhoneNumber";
			szEmail = strchr(szEmail, ':') + 1;
			break;

		case 2:
		case 32:
			szMemberName = "EmailMember";
			szTypeName = "Email";
			szAccIdName = "Email";
			break;
	}

	ezxml_t memb = ezxml_add_child(tbdy, "memberships", 0);
	memb = ezxml_add_child(memb, "Membership", 0);
	node = ezxml_add_child(memb, "MemberRole", 0);
	ezxml_set_txt(node, szRole);
	memb = ezxml_add_child(memb, "Members", 0);
	memb = ezxml_add_child(memb, "Member", 0);
	ezxml_set_attr(memb, "xsi:type",  szMemberName);
	ezxml_set_attr(memb, "xmlns:xsi",  "http://www.w3.org/2001/XMLSchema-instance");
	node = ezxml_add_child(memb, "Type", 0);
	ezxml_set_txt(node, szTypeName);
	node = ezxml_add_child(memb, "State", 0);
	ezxml_set_txt(node, "Accepted");
	node = ezxml_add_child(memb, szAccIdName, 0);
	ezxml_set_txt(node, szEmail);

	char buf[64];
	if ((netId == NETID_LCS || netId == NETID_YAHOO) && strcmp(szMethod, "DeleteMember") != 0)
	{
		node = ezxml_add_child(memb, "Annotations", 0);
		ezxml_t anot = ezxml_add_child(node, "Annotation", 0);
		node = ezxml_add_child(anot, "Name", 0);
		ezxml_set_txt(node, "MSN.IM.BuddyType");
		node = ezxml_add_child(anot, "Value", 0);

		mir_snprintf(buf, sizeof(buf), "%02d:", netId);
		ezxml_set_txt(node, buf);
	}

	char* szData = ezxml_toxml(xmlp, true);

	ezxml_free(xmlp);

	unsigned status;
	char *abUrl = NULL, *tResult;

	for (int k = 4; --k;)
	{
		mir_free(abUrl);
		abUrl = GetABHost(szMethod, true);
		tResult = getSslResult(&abUrl, szData, reqHdr, status);
		if (tResult == NULL) UpdateABHost(szMethod, NULL);
		else break;
	}

	mir_free(reqHdr);
	free(szData);

	if (tResult != NULL)
	{
		UpdateABHost(szMethod, abUrl);
		ezxml_t xmlm = ezxml_parse_str(tResult, strlen(tResult));
		UpdateABCacheKey(xmlm, true);
		if (status == 500)
		{
			const char* szErr = ezxml_txt(getSoapFault(xmlm, true));
			if (strcmp(szErr, "PassportAuthFail") == 0 && allowRecurse)
			{
				MSN_GetPassportAuth();
				status = MSN_SharingAddDelMember(szEmail, listId, netId, szMethod, false) ? 200 : 500;
			}
		}
		ezxml_free(xmlm);
	}

	mir_free(tResult);
	mir_free(abUrl);

	return status == 200;
}

bool CMsnProto::MSN_SharingMyProfile(bool allowRecurse)
{
	char* reqHdr;
	ezxml_t tbdy;
	ezxml_t xmlp = abSoapHdr("AddMember", "RoamingSeed", tbdy, reqHdr);

	ezxml_t svchnd = ezxml_add_child(tbdy, "serviceHandle", 0);
	ezxml_t node = ezxml_add_child(svchnd, "Id", 0);
	ezxml_set_txt(node, "0");
	node = ezxml_add_child(svchnd, "Type", 0);
	ezxml_set_txt(node, "Profile");
	node = ezxml_add_child(svchnd, "ForeignId", 0);
	ezxml_set_txt(node, "MyProfile");

	ezxml_t memb = ezxml_add_child(tbdy, "memberships", 0);
	memb = ezxml_add_child(memb, "Membership", 0);
	node = ezxml_add_child(memb, "MemberRole", 0);
	ezxml_set_txt(node, "ProfileExpression");
	memb = ezxml_add_child(memb, "Members", 0);
	memb = ezxml_add_child(memb, "Member", 0);
	ezxml_set_attr(memb, "xsi:type", "RoleMember");
	ezxml_set_attr(memb, "xmlns:xsi",  "http://www.w3.org/2001/XMLSchema-instance");
	node = ezxml_add_child(memb, "Type", 0);
	ezxml_set_txt(node, "Role");
	node = ezxml_add_child(memb, "State", 0);
	ezxml_set_txt(node, "Accepted");
	node = ezxml_add_child(memb, "Id", 0);
	ezxml_set_txt(node, "Allow");

	ezxml_t svcdef = ezxml_add_child(memb, "DefiningService", 0);
	node = ezxml_add_child(svcdef, "Id", 0);
	ezxml_set_txt(node, "0");
	node = ezxml_add_child(svcdef, "Type", 0);
	ezxml_set_txt(node, "Messenger");
	node = ezxml_add_child(svcdef, "ForeignId", 0);

	node = ezxml_add_child(memb, "MaxRoleRecursionDepth", 0);
	ezxml_set_txt(node, "0");

	node = ezxml_add_child(memb, "MaxDegreesSeparationDepth", 0);
	ezxml_set_txt(node, "0");

	char* szData = ezxml_toxml(xmlp, true);

	ezxml_free(xmlp);

	unsigned status = 0;
	char *abUrl = NULL, *tResult = NULL;

	for (int k = 4; --k;)
	{
		mir_free(abUrl);
		abUrl = GetABHost("AddMember", true);
		tResult = getSslResult(&abUrl, szData, reqHdr, status);
		if (tResult == NULL) UpdateABHost("AddMember", NULL);
		else break;
	}

	mir_free(reqHdr);
	free(szData);

	ezxml_t xmlm = ezxml_parse_str(tResult, strlen(tResult));
	if (status == 500)
	{
		const char* szErr = ezxml_txt(getSoapFault(xmlm, true));
		if (strcmp(szErr, "PassportAuthFail") == 0 && allowRecurse)
		{
			MSN_GetPassportAuth();
			MSN_SharingMyProfile(false);
		}
	}
	ezxml_free(xmlm);

	mir_free(tResult);
	mir_free(abUrl);

	return status == 200;
}


void CMsnProto::SetAbParam(MCONTACT hContact, const char *name, const char *par)
{
	if (*par) setStringUtf(hContact, name, (char*)par);
//	else delSetting(hContact, "FirstName");
}

//		"ABFindAll", "ABFindByContacts", "ABFindContactsPaged"
bool CMsnProto::MSN_ABFind(const char* szMethod, const char* szGuid, bool deltas, bool allowRecurse)
{
	char* reqHdr;
	ezxml_t tbdy;
	ezxml_t xmlp = abSoapHdr(szMethod, "Initial", tbdy, reqHdr);


	const char *szLastChange = NULL;
	if (deltas)
	{
		DBVARIANT dbv;
		if (!getString("ABFullLastChange", &dbv) && dbv.pszVal[0])
		{
			szLastChange = NEWSTR_ALLOCA(dbv.pszVal);
			db_free(&dbv);
		}
		deltas &= (szLastChange != NULL);
	}
	const char *szDynLastChange = NULL;
	if (deltas)
	{
		DBVARIANT dbv;
		if (!getString("ABFullDynLastChange", &dbv) && dbv.pszVal[0])
		{
			szDynLastChange = NEWSTR_ALLOCA(dbv.pszVal);
			db_free(&dbv);
		}
		deltas &= (szDynLastChange != NULL);
	}

	const char *szGroups, *szContacts, *szLastChangeStr;
	if (strcmp(szMethod, "ABFindContactsPaged"))
	{
		ezxml_t node = ezxml_add_child(tbdy, "abView", 0);
		ezxml_set_txt(node, "Full");
		node = ezxml_add_child(tbdy, "deltasOnly", 0);
		ezxml_set_txt(node, deltas ? "true" : "false");
		node = ezxml_add_child(tbdy, "dynamicItemView", 0);
		ezxml_set_txt(node, "Gleam");
		if (deltas)
		{
			node = ezxml_add_child(tbdy, "lastChange", 0);
			ezxml_set_txt(node, szLastChange);
			node = ezxml_add_child(tbdy, "dynamicItemLastChange", 0);
			ezxml_set_txt(node, szDynLastChange);
		}

		if (szGuid)
		{
			node = ezxml_add_child(tbdy, "contactIds", 0);
			node = ezxml_add_child(node, "guid", 0);
			ezxml_set_txt(node, szGuid);
		}
		szGroups = "groups";
		szContacts = "contacts";
		szLastChangeStr = "LastChange";
	}
	else
	{
		ezxml_t node = ezxml_add_child(tbdy, "abView", 0);
		ezxml_set_txt(node, "MessengerClient8");
		node = ezxml_add_child(tbdy, "extendedContent", 0);
		ezxml_set_txt(node, "AB AllGroups CircleResult");
		ezxml_t filt = ezxml_add_child(tbdy, "filterOptions", 0);

		node = ezxml_add_child(filt, "DeltasOnly", 0);
		ezxml_set_txt(node, deltas ? "true" : "false");
		if (deltas)
		{
			node = ezxml_add_child(filt, "LastChanged", 0);
			ezxml_set_txt(node, szLastChange);
		}
 		node = ezxml_add_child(filt, "ContactFilter", 0);
		node = ezxml_add_child(node, "IncludeHiddenContacts", 0);
		ezxml_set_txt(node, "true");

		szGroups = "Groups";
		szContacts = "Contacts";
		szLastChangeStr = "lastChange";
	}

	char* szData = ezxml_toxml(xmlp, true);
	ezxml_free(xmlp);

	unsigned status = 0;
	char *abUrl = NULL, *tResult = NULL;

	for (int k = 4; --k;)
	{
		mir_free(abUrl);
		abUrl = GetABHost(szMethod, false);
		tResult = getSslResult(&abUrl, szData, reqHdr, status);
		if (tResult == NULL) UpdateABHost(szMethod, NULL);
		else break;
	}

	mir_free(reqHdr);
	free(szData);

	if (tResult != NULL)
	{
		ezxml_t xmlm = ezxml_parse_str(tResult, strlen(tResult));
		UpdateABCacheKey(xmlm, false);

		if (status == 200)
		{
			ezxml_t body = getSoapResponse(xmlm, szMethod);
			UpdateABHost(szMethod, body ? abUrl : NULL);

			ezxml_t ab = ezxml_child(body, "Ab");
			if (strcmp(szMethod, "ABFindByContacts"))
			{
				const char* szLastChange = ezxml_txt(ezxml_child(ab, szLastChangeStr));
				if (szLastChange[0]) setString("ABFullLastChange", szLastChange);
				szLastChange = ezxml_txt(ezxml_child(ab, "DynamicItemLastChanged"));
				if (szLastChange[0]) setString("ABFullDynLastChange", szLastChange);
			}

			ezxml_t abinf = ezxml_child(ab, "abInfo");
			mir_snprintf(mycid,  sizeof(mycid), "%s", ezxml_txt(ezxml_child(abinf, "OwnerCID")));
			mir_snprintf(mypuid, sizeof(mycid), "%s", ezxml_txt(ezxml_child(abinf, "ownerPuid")));

			if (MyOptions.ManageServer)
			{
				ezxml_t grp = ezxml_get(body, szGroups, 0, "Group", -1);
				while (grp != NULL)
				{
					const char* szGrpId = ezxml_txt(ezxml_child(grp, "groupId"));
					const char* szGrpName = ezxml_txt(ezxml_get(grp, "groupInfo", 0, "name", -1));
					MSN_AddGroup(szGrpName, szGrpId, true);

					grp = ezxml_next(grp);
				}
			}

			for (ezxml_t cont = ezxml_get(body, szContacts, 0, "Contact", -1); cont != NULL; cont = ezxml_next(cont))
			{
				const char* szContId = ezxml_txt(ezxml_child(cont, "contactId"));

				ezxml_t contInf = ezxml_child(cont, "contactInfo");
				const char* szType = ezxml_txt(ezxml_child(contInf, "contactType"));

				if (strcmp(szType, "Me") != 0)
				{
					char email[128];

					const char* szEmail = ezxml_txt(ezxml_child(contInf, "passportName"));
					const char* szMsgUsr = ezxml_txt(ezxml_child(contInf, "isMessengerUser"));

					int netId = NETID_UNKNOWN;
					if (strcmp(szMsgUsr, "true") == 0) netId = NETID_MSN;

					if (szEmail[0] == '\0')
					{
						ezxml_t eml = ezxml_get(contInf, "emails", 0, "ContactEmail", -1);
						while (eml != NULL)
						{
							szMsgUsr = ezxml_txt(ezxml_child(eml, "isMessengerEnabled"));
							if (strcmp(szMsgUsr, "true") == 0)
							{
								szEmail = ezxml_txt(ezxml_child(eml, "email"));
								const char* szCntType = ezxml_txt(ezxml_child(eml, "contactEmailType"));
								if (strcmp(szCntType, "Messenger2") == 0)
									netId = NETID_YAHOO;
								else if (strcmp(szCntType, "Messenger3") == 0)
									netId = NETID_LCS;
								break;
							}
							eml = ezxml_next(eml);
						}

						if (netId == NETID_UNKNOWN)
						{
							ezxml_t phn = ezxml_get(contInf, "phones", 0, "ContactPhone", -1);
							while (phn != NULL)
							{
								szMsgUsr = ezxml_txt(ezxml_child(phn, "isMessengerEnabled"));
								if (strcmp(szMsgUsr, "true") == 0)
								{
									szEmail = ezxml_txt(ezxml_child(phn, "number"));
									mir_snprintf(email, sizeof(email), "tel:%s", szEmail);
									szEmail = email;
									netId = NETID_MOB;
									break;
								}
								phn = ezxml_next(phn);
							}
						}
					}

					if (netId == NETID_UNKNOWN || szEmail[0] == 0) continue;

					Lists_Add(LIST_FL, netId, szEmail);
					const char *szTmp;

	//				Depricated in WLM 8.1
	//				const char* szNick  = ezxml_txt(ezxml_child(contInf, "displayName"));
	//				if (*szNick == '\0') szNick = szEmail;
					MCONTACT hContact = MSN_HContactFromEmail(szEmail, szEmail, true, false);
	//				setStringUtf(hContact, "Nick", (char*)szNick);

					if (MyOptions.ManageServer)
					{
						ezxml_t grpid = ezxml_child(contInf, "groupIds");
						if (!deltas || grpid)
						{
							ezxml_t grps = ezxml_child(grpid, "guid");
							MSN_SyncContactToServerGroup(hContact, szContId, grps);
						}
					}

					const char* szNick = NULL;
					ezxml_t anot = ezxml_get(contInf, "annotations", 0, "Annotation", -1);
					while (anot != NULL)
					{
						if (strcmp(ezxml_txt(ezxml_child(anot, "Name")), "AB.NickName") == 0)
						{
							szNick = ezxml_txt(ezxml_child(anot, "Value"));
							db_set_utf(hContact, "CList", "MyHandle", szNick);
						}
						if (strcmp(ezxml_txt(ezxml_child(anot, "Name")), "AB.JobTitle") == 0)
						{
							szTmp = ezxml_txt(ezxml_child(anot, "Value"));
							SetAbParam(hContact, "CompanyPosition", szTmp);
						}
						anot = ezxml_next(anot);
					}
					if (szNick == NULL)
						db_unset(hContact, "CList", "MyHandle");

					setString(hContact, "ID", szContId);

					switch (netId)
					{
					case NETID_YAHOO:
						setString(hContact, "Transport", "YAHOO");
						break;

					case NETID_LCS:
						setString(hContact, "Transport", "LCS");
						break;

					default:
						delSetting(hContact, "Transport");
					}

					szTmp = ezxml_txt(ezxml_child(contInf, "CID"));
					SetAbParam(hContact, "CID", szTmp);

					szTmp  = ezxml_txt(ezxml_child(contInf, "IsNotMobileVisible"));
					setByte(hContact, "MobileAllowed", strcmp(szTmp, "true") != 0);

					szTmp = ezxml_txt(ezxml_child(contInf, "isMobileIMEnabled"));
					setByte(hContact, "MobileEnabled", strcmp(szTmp, "true") == 0);

					szTmp = ezxml_txt(ezxml_child(contInf, "firstName"));
					SetAbParam(hContact, "FirstName", szTmp);

					szTmp = ezxml_txt(ezxml_child(contInf, "lastName"));
					SetAbParam(hContact, "LastName", szTmp);

					szTmp = ezxml_txt(ezxml_child(contInf, "birthdate"));
					char *szPtr;
					if (strtol(szTmp, &szPtr, 10) > 1)
					{
						setWord(hContact, "BirthYear", (WORD)strtol(szTmp, &szPtr, 10));
						setByte(hContact, "BirthMonth", (BYTE)strtol(szPtr+1, &szPtr, 10));
						setByte(hContact, "BirthDay", (BYTE)strtol(szPtr+1, &szPtr, 10));
					}
					else
					{
	//					delSetting(hContact, "BirthYear");
	//					delSetting(hContact, "BirthMonth");
	//					delSetting(hContact, "BirthDay");
					}

					szTmp = ezxml_txt(ezxml_child(contInf, "comment"));
					if (*szTmp) db_set_s(hContact, "UserInfo", "MyNotes", szTmp);
	//				else db_unset(hContact, "UserInfo", "MyNotes");

					ezxml_t loc = ezxml_get(contInf, "locations", 0, "ContactLocation", -1);
					while (loc != NULL)
					{
						const char* szCntType = ezxml_txt(ezxml_child(loc, "contactLocationType"));

						int locid = -1;
						if (strcmp(szCntType, "ContactLocationPersonal") == 0)
							locid = 0;
						else if (strcmp(szCntType, "ContactLocationBusiness") == 0)
							locid = 1;

						if (locid >= 0)
						{
							szTmp = ezxml_txt(ezxml_child(loc, "name"));
							SetAbParam(hContact, "Company", szTmp);
							szTmp = ezxml_txt(ezxml_child(loc, "street"));
							SetAbParam(hContact, locid ? "CompanyStreet" : "Street", szTmp);
							szTmp = ezxml_txt(ezxml_child(loc, "city"));
							SetAbParam(hContact, locid ? "CompanyCity" : "City", szTmp);
							szTmp = ezxml_txt(ezxml_child(loc, "state"));
							SetAbParam(hContact, locid ? "CompanyState" : "State", szTmp);
							szTmp = ezxml_txt(ezxml_child(loc, "country"));
							SetAbParam(hContact, locid ? "CompanyCountry" : "Country", szTmp);
							szTmp = ezxml_txt(ezxml_child(loc, "postalCode"));
							SetAbParam(hContact, locid ? "CompanyZIP" : "ZIP", szTmp);
						}
						loc = ezxml_next(loc);
					}

					ezxml_t web = ezxml_get(contInf, "webSites", 0, "ContactWebSite", -1);
					while (web != NULL)
					{
						const char* szCntType = ezxml_txt(ezxml_child(web, "contactWebSiteType"));
						if (strcmp(szCntType, "ContactWebSiteBusiness") == 0)
						{
							szTmp = ezxml_txt(ezxml_child(web, "webURL"));
							SetAbParam(hContact, "CompanyHomepage", szTmp);
						}
						web = ezxml_next(web);
					}
				}
				else
				{
	//              This depricated in WLM 8.1
	//				if (!getByte("NeverUpdateNickname", 0))
	//				{
	//					const char* szNick  = ezxml_txt(ezxml_child(contInf, "displayName"));
	//					setStringUtf(NULL, "Nick", (char*)szNick);
	//				}
					const char *szTmp;

					szTmp = ezxml_txt(ezxml_child(contInf, "isMobileIMEnabled"));
					setByte("MobileEnabled", strcmp(szTmp, "true") == 0);

					szTmp = ezxml_txt(ezxml_child(contInf, "IsNotMobileVisible"));
					setByte("MobileAllowed", strcmp(szTmp, "true") != 0);

					szTmp = ezxml_txt(ezxml_child(contInf, "firstName"));
					setStringUtf(NULL, "FirstName", szTmp);

					szTmp = ezxml_txt(ezxml_child(contInf, "lastName"));
					setStringUtf(NULL, "LastName", szTmp);

					ezxml_t anot = ezxml_get(contInf, "annotations", 0, "Annotation", -1);
					while (anot != NULL)
					{
						if (strcmp(ezxml_txt(ezxml_child(anot, "Name")), "MSN.IM.BLP") == 0)
							msnOtherContactsBlocked = !atol(ezxml_txt(ezxml_child(anot, "Value")));

						anot = ezxml_next(anot);
					}
				}
			}
			if (!msnLoggedIn && msnNsThread)
			{
				char *szCircleTicket = ezxml_txt(ezxml_get(body, "CircleResult", 0, "CircleTicket", -1));
				ptrA szCircleTicketEnc( mir_base64_encode((PBYTE)szCircleTicket, (unsigned)strlen(szCircleTicket)));
				if (szCircleTicketEnc)
					msnNsThread->sendPacket("USR", "SHA A %s", szCircleTicketEnc);
			}

		}
		else if (status == 500)
		{
			const char* szErr = ezxml_txt(getSoapFault(xmlm, true));
			if (strcmp(szErr, "PassportAuthFail") == 0 && allowRecurse)
			{
				MSN_GetPassportAuth();
				status = MSN_ABFind(szMethod, szGuid, deltas, false) ? 200 : 500;
			}
			else if (strcmp(szErr, "FullSyncRequired") == 0 && deltas)
			{
				status = MSN_ABFind(szMethod, szGuid, false, false) ? 200 : 500;
			}
		}
		else
			UpdateABHost(szMethod, NULL);

		ezxml_free(xmlm);
	}
	mir_free(tResult);
	mir_free(abUrl);

	return status == 200;
}


//		"ABGroupContactAdd" : "ABGroupContactDelete", "ABGroupDelete", "ABContactDelete"
bool CMsnProto::MSN_ABAddDelContactGroup(const char* szCntId, const char* szGrpId, const char* szMethod, bool allowRecurse)
{
	char* reqHdr;
	ezxml_t tbdy, node;
	ezxml_t xmlp = abSoapHdr(szMethod, "Timer", tbdy, reqHdr);

	if (szGrpId != NULL)
	{
		node = ezxml_add_child(tbdy, "groupFilter", 0);
		node = ezxml_add_child(node, "groupIds", 0);
		node = ezxml_add_child(node, "guid", 0);
		ezxml_set_txt(node, szGrpId);
	}

	if (szCntId != NULL)
	{
		node = ezxml_add_child(tbdy, "contacts", 0);
		node = ezxml_add_child(node, "Contact", 0);
		node = ezxml_add_child(node, "contactId", 0);
		ezxml_set_txt(node, szCntId);
	}

	char* szData = ezxml_toxml(xmlp, true);
	ezxml_free(xmlp);

	unsigned status = 0;
	char *abUrl = NULL, *tResult = NULL;

	for (int k = 4; --k;)
	{
		mir_free(abUrl);
		abUrl = GetABHost(szMethod, false);
		tResult = getSslResult(&abUrl, szData, reqHdr, status);
		if (tResult == NULL) UpdateABHost(szMethod, NULL);
		else break;
	}

	mir_free(reqHdr);
	free(szData);

	if (tResult != NULL)
	{
		UpdateABHost(szMethod, abUrl);
		ezxml_t xmlm = ezxml_parse_str(tResult, strlen(tResult));
		UpdateABCacheKey(xmlm, false);
		if (status == 500)
		{
			const char* szErr = ezxml_txt(getSoapFault(xmlm, true));
			if (strcmp(szErr, "PassportAuthFail") == 0 && allowRecurse)
			{
				MSN_GetPassportAuth();
				status = MSN_ABAddDelContactGroup(szCntId, szGrpId, szMethod, false) ? 200 : 500;
			}
		}
		ezxml_free(xmlm);
	}
	mir_free(tResult);
	mir_free(abUrl);

	return status == 200;
}

void CMsnProto::MSN_ABAddGroup(const char* szGrpName, bool allowRecurse)
{
	char* reqHdr;
	ezxml_t tbdy;
	ezxml_t xmlp = abSoapHdr("ABGroupAdd", "GroupSave", tbdy, reqHdr);

	ezxml_t node = ezxml_add_child(tbdy, "groupAddOptions", 0);
	node = ezxml_add_child(node, "fRenameOnMsgrConflict", 0);
	ezxml_set_txt(node, "false");

	node = ezxml_add_child(tbdy, "groupInfo", 0);
	ezxml_t grpi = ezxml_add_child(node, "GroupInfo", 0);
	node = ezxml_add_child(grpi, "name", 0);
	ezxml_set_txt(node, szGrpName);
	node = ezxml_add_child(grpi, "groupType", 0);
	ezxml_set_txt(node, "C8529CE2-6EAD-434d-881F-341E17DB3FF8");
	node = ezxml_add_child(grpi, "fMessenger", 0);
	ezxml_set_txt(node, "false");
	node = ezxml_add_child(grpi, "annotations", 0);
	ezxml_t annt = ezxml_add_child(node, "Annotation", 0);
	node = ezxml_add_child(annt, "Name", 0);
	ezxml_set_txt(node, "MSN.IM.Display");
	node = ezxml_add_child(annt, "Value", 0);
	ezxml_set_txt(node, "1");

	char* szData = ezxml_toxml(xmlp, true);
	ezxml_free(xmlp);

	unsigned status = 0;
	char *abUrl = NULL, *tResult = NULL;

	for (int k = 4; --k;)
	{
		mir_free(abUrl);
		abUrl = GetABHost("ABGroupAdd", false);
		tResult = getSslResult(&abUrl, szData, reqHdr, status);
		if (tResult == NULL) UpdateABHost("ABGroupAdd", NULL);
		else break;
	}

	free(szData);
	mir_free(reqHdr);

	if (tResult != NULL)
	{
		UpdateABHost("ABGroupAdd", abUrl);
		ezxml_t xmlm = ezxml_parse_str(tResult, strlen(tResult));
		UpdateABCacheKey(xmlm, false);
		if (status == 200)
		{
			ezxml_t body = getSoapResponse(xmlm, "ABGroupAdd");
			const char* szGrpId = ezxml_txt(ezxml_child(body, "guid"));
			MSN_AddGroup(szGrpName, szGrpId, false);
		}
		else if (status == 500)
		{
			const char* szErr = ezxml_txt(getSoapFault(xmlm, true));
			if (strcmp(szErr, "PassportAuthFail") == 0 && allowRecurse)
			{
				MSN_GetPassportAuth();
				MSN_ABAddGroup(szGrpName, false);
			}
		}
		ezxml_free(xmlm);
	}
	mir_free(tResult);
	mir_free(abUrl);
}


void CMsnProto::MSN_ABRenameGroup(const char* szGrpName, const char* szGrpId, bool allowRecurse)
{
	char* reqHdr;
	ezxml_t tbdy;
	ezxml_t xmlp = abSoapHdr("ABGroupUpdate", "Timer", tbdy, reqHdr);

	ezxml_t node = ezxml_add_child(tbdy, "groups", 0);
	ezxml_t grp = ezxml_add_child(node, "Group", 0);

	node = ezxml_add_child(grp, "groupId", 0);
	ezxml_set_txt(node, szGrpId);
	ezxml_t grpi = ezxml_add_child(grp, "groupInfo", 0);
	node = ezxml_add_child(grpi, "name", 0);
	ezxml_set_txt(node, szGrpName);
	node = ezxml_add_child(grp, "propertiesChanged", 0);
	ezxml_set_txt(node, "GroupName");

	char* szData = ezxml_toxml(xmlp, true);
	ezxml_free(xmlp);

	unsigned status = 0;
	char *abUrl = NULL, *tResult = NULL;

	for (int k = 4; --k;)
	{
		mir_free(abUrl);
		abUrl = GetABHost("ABGroupUpdate", false);
		tResult = getSslResult(&abUrl, szData, reqHdr, status);
		if (tResult == NULL) UpdateABHost("ABGroupUpdate", NULL);
		else break;
	}

	mir_free(reqHdr);
	free(szData);

	if (tResult != NULL)
	{
		UpdateABHost("ABGroupUpdate", abUrl);
		ezxml_t xmlm = ezxml_parse_str(tResult, strlen(tResult));
		UpdateABCacheKey(xmlm, false);
		if (status == 500)
		{
			const char* szErr = ezxml_txt(getSoapFault(xmlm, true));
			if (strcmp(szErr, "PassportAuthFail") == 0 && allowRecurse)
			{
				MSN_GetPassportAuth();
				MSN_ABRenameGroup(szGrpName, szGrpId, false);
			}
		}
		ezxml_free(xmlm);
	}
	mir_free(tResult);
	mir_free(abUrl);
}


bool CMsnProto::MSN_ABAddRemoveContact(const char* szCntId, int netId, bool add, bool allowRecurse)
{
	char* reqHdr;
	ezxml_t tbdy;
	ezxml_t xmlp = abSoapHdr("ABContactUpdate", "Timer", tbdy, reqHdr);

	ezxml_t node = ezxml_add_child(tbdy, "contacts", 0);
	ezxml_t cont = ezxml_add_child(node, "Contact", 0);
	ezxml_set_attr(cont, "xmlns", "http://www.msn.com/webservices/AddressBook");

	node = ezxml_add_child(cont, "contactId", 0);
	ezxml_set_txt(node, szCntId);
	ezxml_t conti = ezxml_add_child(cont, "contactInfo", 0);

	switch (netId)
	{
	case NETID_MSN:
		node = ezxml_add_child(conti, "isMessengerUser", 0);
		ezxml_set_txt(node, add ? "true" : "false");
		node = ezxml_add_child(cont, "propertiesChanged", 0);
		ezxml_set_txt(node, "IsMessengerUser");
		break;

	case NETID_LCS:
	case NETID_YAHOO:
		{
			ezxml_t contp = ezxml_add_child(conti, "emails", 0);
			contp = ezxml_add_child(contp, "ContactEmail", 0);
			node = ezxml_add_child(contp, "contactEmailType", 0);
			ezxml_set_txt(node, netId == NETID_YAHOO ? "Messenger2" : "Messenger3");
			node = ezxml_add_child(contp, "isMessengerEnabled", 0);
			ezxml_set_txt(node, add ? "true" : "false");
			node = ezxml_add_child(contp, "propertiesChanged", 0);
			ezxml_set_txt(node, "IsMessengerEnabled");
			node = ezxml_add_child(cont, "propertiesChanged", 0);
			ezxml_set_txt(node, "ContactEmail");
		}
		break;

	case NETID_MOB:
		{
			ezxml_t contp = ezxml_add_child(conti, "phones", 0);
			contp = ezxml_add_child(contp, "ContactPhone", 0);
			node = ezxml_add_child(contp, "contactPhoneType", 0);
			ezxml_set_txt(node, "ContactPhoneMobile");
			node = ezxml_add_child(contp, "isMessengerEnabled", 0);
			ezxml_set_txt(node, add ? "true" : "false");
			node = ezxml_add_child(contp, "propertiesChanged", 0);
			ezxml_set_txt(node, "IsMessengerEnabled");
			node = ezxml_add_child(cont, "propertiesChanged", 0);
			ezxml_set_txt(node, "ContactPhone");
		}
		break;
	}

	char* szData = ezxml_toxml(xmlp, true);
	ezxml_free(xmlp);

	unsigned status = 0;
	char *abUrl = NULL, *tResult = NULL;

	for (int k = 4; --k;)
	{
		mir_free(abUrl);
		abUrl = GetABHost("ABContactUpdate", false);
		tResult = getSslResult(&abUrl, szData, reqHdr, status);
		if (tResult == NULL) UpdateABHost("ABContactUpdate", NULL);
		else break;
	}

	mir_free(reqHdr);
	free(szData);

	if (tResult != NULL)
	{
		UpdateABHost("ABContactUpdate", abUrl);
		ezxml_t xmlm = ezxml_parse_str(tResult, strlen(tResult));
		UpdateABCacheKey(xmlm, false);
		if (status == 500)
		{
			const char* szErr = ezxml_txt(getSoapFault(xmlm, true));
			if (strcmp(szErr, "PassportAuthFail") == 0 && allowRecurse)
			{
				MSN_GetPassportAuth();
				if (MSN_ABAddRemoveContact(szCntId, netId, add, false))
					status = 200;
			}
		}
		ezxml_free(xmlm);
	}
	mir_free(tResult);
	mir_free(abUrl);

	return status == 200;
}


bool CMsnProto::MSN_ABUpdateProperty(const char* szCntId, const char* propName, const char* propValue, bool allowRecurse)
{
	char* reqHdr;
	ezxml_t tbdy;
	ezxml_t xmlp = abSoapHdr("ABContactUpdate", "Timer", tbdy, reqHdr);

	ezxml_t node = ezxml_add_child(tbdy, "contacts", 0);
	ezxml_t cont = ezxml_add_child(node, "Contact", 0);
	ezxml_set_attr(cont, "xmlns", "http://www.msn.com/webservices/AddressBook");

	ezxml_t conti = ezxml_add_child(cont, "contactInfo", 0);
	if (szCntId == NULL)
	{
		node = ezxml_add_child(conti, "contactType", 0);
		ezxml_set_txt(node, "Me");
	}
	else
	{
		node = ezxml_add_child(cont, "contactId", 0);
		ezxml_set_txt(node, szCntId);
	}
	node = ezxml_add_child(conti, propName, 0);
	ezxml_set_txt(node, propValue);

	node = ezxml_add_child(cont, "propertiesChanged", 0);
	char* szPrpChg = mir_strdup(propName);
	*szPrpChg = _toupper(*szPrpChg);
	ezxml_set_txt(node, szPrpChg);

	char* szData = ezxml_toxml(xmlp, true);
	ezxml_free(xmlp);
	mir_free(szPrpChg);

	unsigned status = 0;
	char *abUrl = NULL, *tResult = NULL;

	for (int k = 4; --k;)
	{
		mir_free(abUrl);
		abUrl = GetABHost("ABContactUpdate", false);
		tResult = getSslResult(&abUrl, szData, reqHdr, status);
		if (tResult == NULL) UpdateABHost("ABContactUpdate", NULL);
		else break;
	}

	mir_free(reqHdr);
	free(szData);

	if (tResult != NULL)
	{
		UpdateABHost("ABContactUpdate", abUrl);
		ezxml_t xmlm = ezxml_parse_str(tResult, strlen(tResult));
		UpdateABCacheKey(xmlm, false);
		if (status == 500)
		{
			const char* szErr = ezxml_txt(getSoapFault(xmlm, true));
			if (strcmp(szErr, "PassportAuthFail") == 0 && allowRecurse)
			{
				MSN_GetPassportAuth();
				if (MSN_ABUpdateProperty(szCntId, propName, propValue, false))
					status = 200;
			}
		}
		ezxml_free(xmlm);
	}
	mir_free(tResult);
	mir_free(abUrl);

	return status == 200;
}


void CMsnProto::MSN_ABUpdateAttr(const char* szCntId, const char* szAttr, const char* szValue, bool allowRecurse)
{
	char* reqHdr;
	ezxml_t tbdy;
	ezxml_t xmlp = abSoapHdr("ABContactUpdate", "Timer", tbdy, reqHdr);

	ezxml_t node = ezxml_add_child(tbdy, "contacts", 0);
	ezxml_t cont = ezxml_add_child(node, "Contact", 0);
	ezxml_set_attr(cont, "xmlns", "http://www.msn.com/webservices/AddressBook");
	ezxml_t conti = ezxml_add_child(cont, "contactInfo", 0);
	if (szCntId == NULL)
	{
		node = ezxml_add_child(conti, "contactType", 0);
		ezxml_set_txt(node, "Me");
	}
	else
	{
		node = ezxml_add_child(cont, "contactId", 0);
		ezxml_set_txt(node, szCntId);
	}
	node = ezxml_add_child(conti, "annotations", 0);
	ezxml_t anot = ezxml_add_child(node, "Annotation", 0);
	node = ezxml_add_child(anot, "Name", 0);
	ezxml_set_txt(node, szAttr);
	node = ezxml_add_child(anot, "Value", 0);
	if (szValue != NULL) ezxml_set_txt(node, szValue);

	node = ezxml_add_child(cont, "propertiesChanged", 0);
	ezxml_set_txt(node, "Annotation");

	char* szData = ezxml_toxml(xmlp, true);
	ezxml_free(xmlp);

	unsigned status = 0;
	char *abUrl = NULL, *tResult = NULL;

	for (int k = 4; --k;)
	{
		mir_free(abUrl);
		abUrl = GetABHost("ABContactUpdate", false);
		tResult = getSslResult(&abUrl, szData, reqHdr, status);
		if (tResult == NULL) UpdateABHost("ABContactUpdate", NULL);
		else break;
	}

	mir_free(reqHdr);
	free(szData);

	if (tResult != NULL)
	{
		UpdateABHost("ABContactUpdate", abUrl);
		ezxml_t xmlm = ezxml_parse_str(tResult, strlen(tResult));
		UpdateABCacheKey(xmlm, false);
		if (status == 500)
		{
			const char* szErr = ezxml_txt(getSoapFault(xmlm, true));
			if (strcmp(szErr, "PassportAuthFail") == 0 && allowRecurse)
			{
				MSN_GetPassportAuth();
				MSN_ABUpdateAttr(szCntId, szAttr, szValue, false);
			}
		}
		ezxml_free(xmlm);
	}
	mir_free(tResult);
	mir_free(abUrl);
}


void CMsnProto::MSN_ABUpdateNick(const char* szNick, const char* szCntId)
{
	if (szCntId != NULL)
		MSN_ABUpdateAttr(szCntId, "AB.NickName", szNick);
	else
		MSN_ABUpdateProperty(szCntId, "displayName", szNick);
}


unsigned CMsnProto::MSN_ABContactAdd(const char* szEmail, const char* szNick, int netId, const char* szInvite, bool search, bool retry, bool allowRecurse)
{
	char* reqHdr;
	ezxml_t tbdy;
	ezxml_t xmlp = abSoapHdr("ABContactAdd", "ContactSave", tbdy, reqHdr);

	ezxml_t conts = ezxml_add_child(tbdy, "contacts", 0);
	ezxml_t node = ezxml_add_child(conts, "Contact", 0);
	ezxml_set_attr(node, "xmlns", "http://www.msn.com/webservices/AddressBook");
	ezxml_t conti = ezxml_add_child(node, "contactInfo", 0);
	ezxml_t contp;

	const char* szEmailNP = strchr(szEmail, ':');
	if (szEmailNP != NULL) netId = NETID_MOB;

	switch (netId)
	{
	case NETID_MSN:
		node = ezxml_add_child(conti, "contactType", 0);
		ezxml_set_txt(node, "LivePending");
		node = ezxml_add_child(conti, "passportName", 0);
		ezxml_set_txt(node, szEmail);
		node = ezxml_add_child(conti, "isMessengerUser", 0);
		ezxml_set_txt(node, "true");

		if (szInvite)
		{
			node = ezxml_add_child(conti, "MessengerMemberInfo", 0);
			node = ezxml_add_child(node, "PendingAnnotations", 0);
			ezxml_t anot = ezxml_add_child(node, "Annotation", 0);
			node = ezxml_add_child(anot, "Name", 0);
			ezxml_set_txt(node, "MSN.IM.InviteMessage");
			node = ezxml_add_child(anot, "Value", 0);
			ezxml_set_txt(node, szInvite);
		}
		break;

	case NETID_MOB:
		++szEmailNP;
		if (szNick == NULL) szNick = szEmailNP;
		node = ezxml_add_child(conti, "phones", 0);
		contp = ezxml_add_child(node, "ContactPhone", 0);
		node = ezxml_add_child(contp, "contactPhoneType", 0);
		ezxml_set_txt(node, "ContactPhoneMobile");
		node = ezxml_add_child(contp, "number", 0);
		ezxml_set_txt(node, szEmailNP);
		node = ezxml_add_child(contp, "isMessengerEnabled", 0);
		ezxml_set_txt(node, "true");
		node = ezxml_add_child(contp, "propertiesChanged", 0);
		ezxml_set_txt(node, "Number IsMessengerEnabled");
		break;

	case NETID_LCS:
	case NETID_YAHOO:
		node = ezxml_add_child(conti, "emails", 0);
		contp = ezxml_add_child(node, "ContactEmail", 0);
		node = ezxml_add_child(contp, "contactEmailType", 0);
		ezxml_set_txt(node, netId == NETID_YAHOO ? "Messenger2" : "Messenger3");
		node = ezxml_add_child(contp, "email", 0);
		ezxml_set_txt(node, szEmail);
		node = ezxml_add_child(contp, "isMessengerEnabled", 0);
		ezxml_set_txt(node, "true");
		node = ezxml_add_child(contp, "Capability", 0);
		ezxml_set_txt(node,  netId == NETID_YAHOO ? "32" : "2");
		node = ezxml_add_child(contp, "propertiesChanged", 0);
		ezxml_set_txt(node, "Email IsMessengerEnabled Capability");
		break;
	}

	if (szNick != NULL)
	{
		node = ezxml_add_child(conti, "annotations", 0);
		ezxml_t annt = ezxml_add_child(node, "Annotation", 0);
		node = ezxml_add_child(annt, "Name", 0);
		ezxml_set_txt(node, "MSN.IM.Display");
		node = ezxml_add_child(annt, "Value", 0);
		ezxml_set_txt(node, szNick);
	}

	node = ezxml_add_child(conts, "options", 0);
	node = ezxml_add_child(node, "EnableAllowListManagement", 0);
	ezxml_set_txt(node, "true");

	char* szData = ezxml_toxml(xmlp, true);
	ezxml_free(xmlp);

	unsigned status = 0;
	char *abUrl = NULL, *tResult = NULL;

	for (int k = 4; --k;)
	{
		mir_free(abUrl);
		abUrl = GetABHost("ABContactAdd", false);
		tResult = getSslResult(&abUrl, szData, reqHdr, status);
		if (tResult == NULL) UpdateABHost("ABContactAdd", NULL);
		else break;
	}

	mir_free(reqHdr);
	free(szData);

	if (tResult != NULL)
	{
		ezxml_t xmlm = ezxml_parse_str(tResult, strlen(tResult));
		UpdateABCacheKey(xmlm, false);
		if (status == 200)
		{
			ezxml_t body = getSoapResponse(xmlm, "ABContactAdd");

			const char* szContId = ezxml_txt(ezxml_child(body, "guid"));

			if (search)
				MSN_ABAddDelContactGroup(szContId , NULL, "ABContactDelete");
			else
			{
				MSN_ABAddRemoveContact(szContId, NETID_MSN, true);
				MCONTACT hContact = MSN_HContactFromEmail(szEmail, szNick ? szNick : szEmail, true, false);
				setString(hContact, "ID", szContId);
			}
			status = 0;
		}
		else if (status == 500)
		{
			const char* szErr = ezxml_txt(getSoapFault(xmlm, true));

			if (strcmp(szErr, "InvalidPassportUser") == 0)
				status = 1;
			else if (strcmp(szErr, "FederatedQueryFailure") == 0)
				status = 4;
			else if (strcmp(szErr, "EmailDomainIsFederated") == 0)
				status = 2;
			else if (strcmp(szErr, "BadEmailArgument") == 0)
				status = 4;
			else if (strcmp(szErr, "ContactAlreadyExists") == 0)
			{
				status = 3;

				ezxml_t node = getSoapFault(xmlm, false);
				node = ezxml_get(node, "detail", 0, "additionalDetails", 0, "conflictObjectId", -1);
				const char* szContId = ezxml_txt(node);

				if (search)
				{
					if (retry)
					{
						MSN_ABAddDelContactGroup(szContId , NULL, "ABContactDelete");
						status = 0;
					}
				}
				else
				{
					MCONTACT hContact = MSN_HContactFromEmail(szEmail, szNick ? szNick : szEmail, true, false);
					setString(hContact, "ID", szContId);
				}
			}
			else if (strcmp(szErr, "PassportAuthFail") == 0 && allowRecurse)
			{
				MSN_GetPassportAuth();
				status = MSN_ABContactAdd(szEmail, szNick, netId, NULL, search, retry, false);
			}
			else
			{
				status = MSN_ABContactAdd(szEmail, szNick, netId, NULL, search, false);
			}
		}
		ezxml_free(xmlm);
	}
	mir_free(tResult);
	mir_free(abUrl);

	return status;
}


void CMsnProto::MSN_ABUpdateDynamicItem(bool allowRecurse)
{
	char* reqHdr;
	ezxml_t tbdy;
	ezxml_t xmlp = abSoapHdr("UpdateDynamicItem", "RoamingIdentityChanged", tbdy, reqHdr);

	ezxml_t dynitms = ezxml_add_child(tbdy, "dynamicItems", 0);
	ezxml_t dynitm = ezxml_add_child(dynitms, "DynamicItem", 0);

	ezxml_set_attr(dynitm, "xsi:type", "PassportDynamicItem");
	ezxml_t node = ezxml_add_child(dynitm, "Type", 0);
	ezxml_set_txt(node, "Passport");
	node = ezxml_add_child(dynitm, "PassportName", 0);
	ezxml_set_txt(node, MyOptions.szEmail);

	ezxml_t nots = ezxml_add_child(dynitm, "Notifications", 0);
	ezxml_t notd = ezxml_add_child(nots, "NotificationData", 0);
	ezxml_t strsvc = ezxml_add_child(notd, "StoreService", 0);
	ezxml_t info = ezxml_add_child(strsvc, "Info", 0);

	ezxml_t hnd = ezxml_add_child(info, "Handle", 0);
	node = ezxml_add_child(hnd, "Id", 0);
	ezxml_set_txt(node, "0");
	node = ezxml_add_child(hnd, "Type", 0);
	ezxml_set_txt(node, "Profile");
	node = ezxml_add_child(hnd, "ForeignId", 0);
	ezxml_set_txt(node, "MyProfile");

	node = ezxml_add_child(info, "InverseRequired", 0);
	ezxml_set_txt(node, "false");
	node = ezxml_add_child(info, "IsBot", 0);
	ezxml_set_txt(node, "false");

	node = ezxml_add_child(strsvc, "Changes", 0);
	node = ezxml_add_child(strsvc, "LastChange", 0);
	ezxml_set_txt(node, "0001-01-01T00:00:00");
	node = ezxml_add_child(strsvc, "Deleted", 0);
	ezxml_set_txt(node, "false");

	node = ezxml_add_child(notd, "Status", 0);
	ezxml_set_txt(node, "Exist Access");
	node = ezxml_add_child(notd, "LastChanged", 0);

	time_t timer;
	time(&timer);
	tm *tmst = gmtime(&timer);

	char tmstr[32];
	mir_snprintf(tmstr, sizeof(tmstr), "%04u-%02u-%02uT%02u:%02u:%02uZ",
		tmst->tm_year + 1900, tmst->tm_mon+1, tmst->tm_mday,
		tmst->tm_hour, tmst->tm_min, tmst->tm_sec);

	ezxml_set_txt(node, tmstr);
	node = ezxml_add_child(notd, "Gleam", 0);
	ezxml_set_txt(node, "false");
	node = ezxml_add_child(notd, "InstanceId", 0);
	ezxml_set_txt(node, "0");

	node = ezxml_add_child(dynitm, "Changes", 0);
	ezxml_set_txt(node, "Notifications");

	char* szData = ezxml_toxml(xmlp, true);
	ezxml_free(xmlp);

	unsigned status;
	char *abUrl = NULL, *tResult;

	for (int k = 4; --k;)
	{
		mir_free(abUrl);
		abUrl = GetABHost("UpdateDynamicItem", false);
		tResult = getSslResult(&abUrl, szData, reqHdr, status);
		if (tResult == NULL) UpdateABHost("UpdateDynamicItem", NULL);
		else break;
	}

	mir_free(reqHdr);
	free(szData);

	if (tResult != NULL)
	{
		UpdateABHost("UpdateDynamicItem", abUrl);
		ezxml_t xmlm = ezxml_parse_str(tResult, strlen(tResult));
		UpdateABCacheKey(xmlm, false);
		if (status == 500)
		{
			const char* szErr = ezxml_txt(getSoapFault(xmlm, true));
			if (strcmp(szErr, "PassportAuthFail") == 0 && allowRecurse)
			{
				MSN_GetPassportAuth();
				MSN_ABUpdateDynamicItem(false);
			}
		}
		ezxml_free(xmlm);
	}
	mir_free(tResult);
	mir_free(abUrl);
}
