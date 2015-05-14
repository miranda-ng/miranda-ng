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

static const char storeReqHdr[] =
	"SOAPAction: http://www.msn.com/webservices/storage/2008/%s\r\n";

ezxml_t CMsnProto::storeSoapHdr(const char* service, const char* scenario, ezxml_t& tbdy, char*& httphdr)
{
	ezxml_t xmlp = ezxml_new("soap:Envelope");
	ezxml_set_attr(xmlp, "xmlns:soap", "http://schemas.xmlsoap.org/soap/envelope/");
	ezxml_set_attr(xmlp, "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	ezxml_set_attr(xmlp, "xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
	ezxml_set_attr(xmlp, "xmlns:soapenc", "http://schemas.xmlsoap.org/soap/encoding/");

	ezxml_t hdr = ezxml_add_child(xmlp, "soap:Header", 0);

	if (storageCacheKey) {
		ezxml_t cachehdr = ezxml_add_child(hdr, "AffinityCacheHeader", 0);
		ezxml_set_attr(cachehdr, "xmlns", "http://www.msn.com/webservices/storage/2008");
		ezxml_t node = ezxml_add_child(cachehdr, "CacheKey", 0);
		ezxml_set_txt(node, storageCacheKey);
	}

	ezxml_t apphdr = ezxml_add_child(hdr, "StorageApplicationHeader", 0);
	ezxml_set_attr(apphdr, "xmlns", "http://www.msn.com/webservices/storage/2008");
	ezxml_t node = ezxml_add_child(apphdr, "ApplicationID", 0);
	ezxml_set_txt(node, "Messenger Client 9.0");
	node = ezxml_add_child(apphdr, "Scenario", 0);
	ezxml_set_txt(node, scenario);

	ezxml_t authhdr = ezxml_add_child(hdr, "StorageUserHeader", 0);
	ezxml_set_attr(authhdr, "xmlns", "http://www.msn.com/webservices/storage/2008");
	node = ezxml_add_child(authhdr, "Puid", 0);
	ezxml_set_txt(node, mypuid);
	node = ezxml_add_child(authhdr, "TicketToken", 0);
	if (authStorageToken) ezxml_set_txt(node, authStorageToken);

	ezxml_t bdy = ezxml_add_child(xmlp, "soap:Body", 0);

	tbdy = ezxml_add_child(bdy, service, 0);
	ezxml_set_attr(tbdy, "xmlns", "http://www.msn.com/webservices/storage/2008");

	size_t hdrsz = strlen(service) + sizeof(storeReqHdr) + 20;
	httphdr = (char*)mir_alloc(hdrsz);

	mir_snprintf(httphdr, hdrsz, storeReqHdr, service);

	return xmlp;
}

char* CMsnProto::GetStoreHost(const char* service)
{
	char hostname[128];
	mir_snprintf(hostname, SIZEOF(hostname), "StoreHost-%s", service);

	char* host = (char*)mir_alloc(256);
	if (db_get_static(NULL, m_szModuleName, hostname, host, 256) || !*host)
		strcpy(host, "https://tkrdr.storage.msn.com/storageservice/SchematizedStore.asmx");

	return host;
}

void CMsnProto::UpdateStoreHost(const char* service, const char* url)
{
	char hostname[128];
	mir_snprintf(hostname, SIZEOF(hostname), "StoreHost-%s", service);

	setString(hostname, url);
}

void CMsnProto::UpdateStoreCacheKey(ezxml_t bdy)
{
	ezxml_t key = ezxml_get(bdy, "soap:Header", 0, "AffinityCacheHeader", 0, "CacheKey", -1);
	if (key) replaceStr(storageCacheKey, ezxml_txt(key));
}

bool CMsnProto::MSN_StoreCreateProfile(bool allowRecurse)
{
	char* reqHdr;
	ezxml_t tbdy;
	ezxml_t xmlp = storeSoapHdr("CreateProfile", "RoamingSeed", tbdy, reqHdr);

	ezxml_t pro = ezxml_add_child(tbdy, "profile", 0);
	ezxml_t node;

	pro = ezxml_add_child(pro, "ExpressionProfile", 0);
	ezxml_add_child(pro, "PersonalStatus", 0);
	node = ezxml_add_child(pro, "RoleDefinitionName", 0);
	ezxml_set_txt(node, "ExpressionProfileDefault");

	char* szData = ezxml_toxml(xmlp, true);

	ezxml_free(xmlp);

	unsigned status = 0;
	char *storeUrl, *tResult = NULL;

	storeUrl = mir_strdup("https://storage.msn.com/storageservice/SchematizedStore.asmx");
	tResult = getSslResult(&storeUrl, szData, reqHdr, status);

	mir_free(reqHdr);
	free(szData);

	if (tResult != NULL) {
		if (status == 200) {
			ezxml_t xmlm = ezxml_parse_str(tResult, strlen(tResult));
			UpdateStoreCacheKey(xmlm);
			ezxml_t body = getSoapResponse(xmlm, "CreateProfile");

			MSN_StoreShareItem(ezxml_txt(body));
			MSN_SharingMyProfile();

			ezxml_free(xmlm);
		}
		else if (status == 500) {
			ezxml_t xmlm = ezxml_parse_str(tResult, strlen(tResult));
			const char* szErr = ezxml_txt(getSoapFault(xmlm, true));
			if (strcmp(szErr, "PassportAuthFail") == 0 && allowRecurse) {
				MSN_GetPassportAuth();
				status = MSN_StoreCreateProfile(false) ? 200 : 500;
			}
			ezxml_free(xmlm);
		}
	}

	mir_free(tResult);
	mir_free(storeUrl);

	return status == 200;
}

bool CMsnProto::MSN_StoreShareItem(const char* id, bool allowRecurse)
{
	char* reqHdr;
	ezxml_t tbdy;
	ezxml_t xmlp = storeSoapHdr("ShareItem", "RoamingSeed", tbdy, reqHdr);

	ezxml_t node = ezxml_add_child(tbdy, "resourceID", 0);
	ezxml_set_txt(node, id);
	node = ezxml_add_child(tbdy, "displayName", 0);
	ezxml_set_txt(node, "Messenger Roaming Identity");

	char* szData = ezxml_toxml(xmlp, true);

	ezxml_free(xmlp);

	unsigned status = 0;
	char *storeUrl, *tResult = NULL;

	storeUrl = mir_strdup("https://storage.msn.com/storageservice/SchematizedStore.asmx");
	tResult = getSslResult(&storeUrl, szData, reqHdr, status);

	mir_free(reqHdr);
	free(szData);

	if (tResult != NULL && status == 500) {
		ezxml_t xmlm = ezxml_parse_str(tResult, strlen(tResult));
		const char* szErr = ezxml_txt(getSoapFault(xmlm, true));
		if (strcmp(szErr, "PassportAuthFail") == 0 && allowRecurse) {
			MSN_GetPassportAuth();
			status = MSN_StoreCreateProfile(false) ? 200 : 500;
		}
		ezxml_free(xmlm);
	}

	mir_free(tResult);
	mir_free(storeUrl);

	return status == 200;
}

bool CMsnProto::MSN_StoreGetProfile(bool allowRecurse)
{
	char* reqHdr;
	ezxml_t tbdy;
	ezxml_t xmlp = storeSoapHdr("GetProfile", "Initial", tbdy, reqHdr);

	ezxml_t prohndl = ezxml_add_child(tbdy, "profileHandle", 0);

	ezxml_t alias = ezxml_add_child(prohndl, "Alias", 0);
	ezxml_t node = ezxml_add_child(alias, "Name", 0);
	ezxml_set_txt(node, mycid);
	node = ezxml_add_child(alias, "NameSpace", 0);
	ezxml_set_txt(node, "MyCidStuff");

	node = ezxml_add_child(prohndl, "RelationshipName", 0);
	ezxml_set_txt(node, "MyProfile");

	ezxml_t proattr = ezxml_add_child(tbdy, "profileAttributes", 0);
	node = ezxml_add_child(proattr, "ResourceID", 0);
	ezxml_set_txt(node, "true");
	node = ezxml_add_child(proattr, "DateModified", 0);
	ezxml_set_txt(node, "true");

	ezxml_t exproattr = ezxml_add_child(proattr, "ExpressionProfileAttributes", 0);
	node = ezxml_add_child(exproattr, "ResourceID", 0);
	ezxml_set_txt(node, "true");
	node = ezxml_add_child(exproattr, "DateModified", 0);
	ezxml_set_txt(node, "true");
	node = ezxml_add_child(exproattr, "DisplayName", 0);
	ezxml_set_txt(node, "true");
	node = ezxml_add_child(exproattr, "DisplayNameLastModified", 0);
	ezxml_set_txt(node, "true");
	node = ezxml_add_child(exproattr, "PersonalStatus", 0);
	ezxml_set_txt(node, "true");
	node = ezxml_add_child(exproattr, "PersonalStatusLastModified", 0);
	ezxml_set_txt(node, "true");
	node = ezxml_add_child(exproattr, "StaticUserTilePublicURL", 0);
	ezxml_set_txt(node, "true");
	node = ezxml_add_child(exproattr, "Photo", 0);
	ezxml_set_txt(node, "true");
	node = ezxml_add_child(exproattr, "Flags", 0);
	ezxml_set_txt(node, "true");

	char* szData = ezxml_toxml(xmlp, true);

	ezxml_free(xmlp);

	unsigned status = 0;
	char *storeUrl = NULL, *tResult = NULL;

	for (int k = 4; --k;) {
		mir_free(storeUrl);
		storeUrl = GetStoreHost("GetProfile");
		tResult = getSslResult(&storeUrl, szData, reqHdr, status);
		if (tResult == NULL) UpdateStoreHost("GetProfile", NULL);
		else break;
	}

	mir_free(reqHdr);
	free(szData);

	if (tResult != NULL) {
		if (status == 200) {
			ezxml_t xmlm = ezxml_parse_str(tResult, strlen(tResult));
			ezxml_t body = getSoapResponse(xmlm, "GetProfile");

			UpdateStoreHost("GetProfile", body ? storeUrl : NULL);

			strncpy_s(proresid, ezxml_txt(ezxml_child(body, "ResourceID")), _TRUNCATE);

			ezxml_t expr = ezxml_child(body, "ExpressionProfile");
			if (expr == NULL) {
				MSN_StoreShareItem(proresid);
				MSN_SharingMyProfile();
				if (allowRecurse) MSN_StoreGetProfile(false);
			}
			else {
				const char* szNick = ezxml_txt(ezxml_child(expr, "DisplayName"));
				setStringUtf(NULL, "Nick", (char*)szNick);

				const char* szStatus = ezxml_txt(ezxml_child(expr, "PersonalStatus"));
				replaceStr(msnLastStatusMsg, szStatus);

				strncpy_s(expresid, ezxml_txt(ezxml_child(expr, "ResourceID")), _TRUNCATE);

				ezxml_t photo = ezxml_child(expr, "Photo");
				strncpy_s(photoid, ezxml_txt(ezxml_child(photo, "ResourceID")), _TRUNCATE);

				ezxml_t docstr = ezxml_get(photo, "DocumentStreams", 0, "DocumentStream", -1);
				while (docstr) {
					const char *docname = ezxml_txt(ezxml_child(docstr, "DocumentStreamName"));
					if (!strcmp(docname, "UserTileStatic")) {
						getMyAvatarFile(ezxml_txt(ezxml_child(docstr, "PreAuthURL")), _T("miranda_avatar.tmp"));
						break;
					}
					docstr = ezxml_next(docstr);
				}
			}
			ezxml_free(xmlm);
		}
		else if (status == 500 && allowRecurse) {
			ezxml_t xmlm = ezxml_parse_str(tResult, strlen(tResult));
			const char* szErr = ezxml_txt(getSoapFault(xmlm, true));
			if (strcmp(szErr, "PassportAuthFail") == 0) {
				MSN_GetPassportAuth();
				MSN_StoreGetProfile(false);
			}
			else {
				MSN_StoreCreateProfile();
				if (MSN_StoreGetProfile(false)) status = 200;
			}
			ezxml_free(xmlm);
		}
		else
			UpdateStoreHost("GetProfile", NULL);

	}
	mir_free(tResult);
	mir_free(storeUrl);

	return status == 200;
}

bool CMsnProto::MSN_StoreUpdateProfile(const char* szNick, const char* szStatus, bool lock, bool allowRecurse)
{
	char* reqHdr;
	ezxml_t tbdy;
	ezxml_t xmlp = storeSoapHdr("UpdateProfile", "RoamingIdentityChanged", tbdy, reqHdr);

	ezxml_t pro = ezxml_add_child(tbdy, "profile", 0);
	ezxml_t node = ezxml_add_child(pro, "ResourceID", 0);
	ezxml_set_txt(node, proresid);

	ezxml_t expro = ezxml_add_child(pro, "ExpressionProfile", 0);
	node = ezxml_add_child(expro, "FreeText", 0);
	ezxml_set_txt(node, "Update");
	if (szNick) {
		node = ezxml_add_child(expro, "DisplayName", 0);
		ezxml_set_txt(node, szNick);
	}
	if (szStatus) {
		node = ezxml_add_child(expro, "PersonalStatus", 0);
		ezxml_set_txt(node, szStatus);
	}
	node = ezxml_add_child(expro, "Flags", 0);
	ezxml_set_txt(node, lock ? "1" : "0");

	char* szData = ezxml_toxml(xmlp, true);

	ezxml_free(xmlp);

	unsigned status = 0;
	char *storeUrl = NULL, *tResult = NULL;

	for (int k = 4; --k;) {
		mir_free(storeUrl);
		storeUrl = GetStoreHost("UpdateProfile");
		tResult = getSslResult(&storeUrl, szData, reqHdr, status);
		if (tResult == NULL) UpdateStoreHost("UpdateProfile", NULL);
		else break;
	}

	mir_free(reqHdr);
	free(szData);

	if (tResult != NULL) {
		UpdateStoreHost("UpdateProfile", storeUrl);
		if (status == 200) {
			replaceStr(msnLastStatusMsg, szStatus);
			MSN_ABUpdateDynamicItem();
		}
		else if (status == 500 && allowRecurse) {
			ezxml_t xmlm = ezxml_parse_str(tResult, strlen(tResult));
			const char* szErr = ezxml_txt(getSoapFault(xmlm, true));
			if (strcmp(szErr, "PassportAuthFail") == 0) {
				MSN_GetPassportAuth();
				status = MSN_StoreUpdateProfile(szNick, szStatus, lock, false) ? 200 : 500;
			}
			ezxml_free(xmlm);
		}
	}

	mir_free(tResult);
	mir_free(storeUrl);

	return status == 200;
}


bool CMsnProto::MSN_StoreCreateRelationships(bool allowRecurse)
{
	char* reqHdr;
	ezxml_t tbdy;
	ezxml_t xmlp = storeSoapHdr("CreateRelationships", "RoamingIdentityChanged", tbdy, reqHdr);

	ezxml_t rels = ezxml_add_child(tbdy, "relationships", 0);
	ezxml_t rel = ezxml_add_child(rels, "Relationship", 0);
	ezxml_t node = ezxml_add_child(rel, "SourceID", 0);
	ezxml_set_txt(node, expresid);
	node = ezxml_add_child(rel, "SourceType", 0);
	ezxml_set_txt(node, "SubProfile");
	node = ezxml_add_child(rel, "TargetID", 0);
	ezxml_set_txt(node, photoid);
	node = ezxml_add_child(rel, "TargetType", 0);
	ezxml_set_txt(node, "Photo");
	node = ezxml_add_child(rel, "RelationshipName", 0);
	ezxml_set_txt(node, "ProfilePhoto");

	char* szData = ezxml_toxml(xmlp, true);

	ezxml_free(xmlp);

	unsigned status = 0;
	char *storeUrl = NULL, *tResult = NULL;

	for (int k = 4; --k;) {
		mir_free(storeUrl);
		storeUrl = GetStoreHost("CreateRelationships");
		tResult = getSslResult(&storeUrl, szData, reqHdr, status);
		if (tResult == NULL) UpdateStoreHost("CreateRelationships", NULL);
		else break;
	}

	mir_free(reqHdr);
	free(szData);

	if (tResult != NULL) {
		UpdateStoreHost("CreateRelationships", storeUrl);

		if (status == 500) {
			ezxml_t xmlm = ezxml_parse_str(tResult, strlen(tResult));
			const char* szErr = ezxml_txt(getSoapFault(xmlm, true));
			if (strcmp(szErr, "PassportAuthFail") == 0 && allowRecurse) {
				MSN_GetPassportAuth();
				status = MSN_StoreCreateRelationships(false) ? 200 : 500;
			}
			ezxml_free(xmlm);
		}
	}

	mir_free(tResult);
	mir_free(storeUrl);

	return status == 200;
}


bool CMsnProto::MSN_StoreDeleteRelationships(bool tile, bool allowRecurse)
{
	char* reqHdr;
	ezxml_t tbdy;
	ezxml_t xmlp = storeSoapHdr("DeleteRelationships", "RoamingIdentityChanged", tbdy, reqHdr);

	ezxml_t srch = ezxml_add_child(tbdy, "sourceHandle", 0);

	ezxml_t node;
	if (tile) {
		node = ezxml_add_child(srch, "RelationshipName", 0);
		ezxml_set_txt(node, "/UserTiles");

		ezxml_t alias = ezxml_add_child(srch, "Alias", 0);
		node = ezxml_add_child(alias, "Name", 0);
		ezxml_set_txt(node, mycid);
		node = ezxml_add_child(alias, "NameSpace", 0);
		ezxml_set_txt(node, "MyCidStuff");
	}
	else {
		node = ezxml_add_child(srch, "ResourceID", 0);
		ezxml_set_txt(node, expresid);
	}

	node = ezxml_add_child(tbdy, "targetHandles", 0);
	node = ezxml_add_child(node, "ObjectHandle", 0);
	node = ezxml_add_child(node, "ResourceID", 0);
	ezxml_set_txt(node, photoid);

	char* szData = ezxml_toxml(xmlp, true);

	ezxml_free(xmlp);

	unsigned status = 0;
	char *storeUrl = NULL, *tResult = NULL;

	for (int k = 4; --k;) {
		mir_free(storeUrl);
		storeUrl = GetStoreHost("DeleteRelationships");
		tResult = getSslResult(&storeUrl, szData, reqHdr, status);
		if (tResult == NULL) UpdateStoreHost("DeleteRelationships", NULL);
		else break;
	}

	mir_free(reqHdr);
	free(szData);

	if (tResult != NULL) {
		UpdateStoreHost("DeleteRelationships", storeUrl);
		if (status == 500) {
			ezxml_t xmlm = ezxml_parse_str(tResult, strlen(tResult));
			const char* szErr = ezxml_txt(getSoapFault(xmlm, true));
			if (strcmp(szErr, "PassportAuthFail") == 0 && allowRecurse) {
				MSN_GetPassportAuth();
				status = MSN_StoreDeleteRelationships(tile, false) ? 200 : 500;
			}
			ezxml_free(xmlm);
		}
	}

	mir_free(tResult);
	mir_free(storeUrl);

	return status == 200;
}


bool CMsnProto::MSN_StoreCreateDocument(const TCHAR *sztName, const char *szMimeType, const char *szPicData, bool allowRecurse)
{
	char* reqHdr;
	ezxml_t tbdy;
	char* szName = mir_utf8encodeT(sztName);
	ezxml_t xmlp = storeSoapHdr("CreateDocument", "RoamingIdentityChanged", tbdy, reqHdr);

	ezxml_t hndl = ezxml_add_child(tbdy, "parentHandle", 0);
	ezxml_t node = ezxml_add_child(hndl, "RelationshipName", 0);
	ezxml_set_txt(node, "/UserTiles");

	ezxml_t alias = ezxml_add_child(hndl, "Alias", 0);
	node = ezxml_add_child(alias, "Name", 0);
	ezxml_set_txt(node, mycid);
	node = ezxml_add_child(alias, "NameSpace", 0);
	ezxml_set_txt(node, "MyCidStuff");

	ezxml_t doc = ezxml_add_child(tbdy, "document", 0);
	ezxml_set_attr(doc, "xsi:type", "Photo");
	node = ezxml_add_child(doc, "Name", 0);
	ezxml_set_txt(node, szName);

	doc = ezxml_add_child(doc, "DocumentStreams", 0);
	doc = ezxml_add_child(doc, "DocumentStream", 0);
	ezxml_set_attr(doc, "xsi:type", "PhotoStream");
	node = ezxml_add_child(doc, "DocumentStreamType", 0);

	ezxml_set_txt(node, "UserTileStatic");
	node = ezxml_add_child(doc, "MimeType", 0);
	ezxml_set_txt(node, szMimeType);
	node = ezxml_add_child(doc, "Data", 0);
	ezxml_set_txt(node, szPicData);
	node = ezxml_add_child(doc, "DataSize", 0);
	ezxml_set_txt(node, "0");

	node = ezxml_add_child(tbdy, "relationshipName", 0);
	ezxml_set_txt(node, "Messenger User Tile");

	char* szData = ezxml_toxml(xmlp, true);

	ezxml_free(xmlp);
	mir_free(szName);

	unsigned status = 0;
	char *storeUrl = NULL, *tResult = NULL;

	for (int k = 4; --k;) {
		mir_free(storeUrl);
		storeUrl = GetStoreHost("CreateDocument");
		tResult = getSslResult(&storeUrl, szData, reqHdr, status);
		if (tResult == NULL) UpdateStoreHost("CreateDocument", NULL);
		else break;
	}

	mir_free(reqHdr);
	free(szData);

	if (tResult != NULL) {
		UpdateStoreHost("CreateDocument", storeUrl);
		if (status == 200) {
			ezxml_t xmlm = ezxml_parse_str(tResult, strlen(tResult));
			ezxml_t bdy = getSoapResponse(xmlm, "CreateDocument");
			strncpy_s(photoid, ezxml_txt(bdy), _TRUNCATE);
			ezxml_free(xmlm);
		}
		else if (status == 500) {
			ezxml_t xmlm = ezxml_parse_str(tResult, strlen(tResult));
			const char* szErr = ezxml_txt(getSoapFault(xmlm, true));
			if (strcmp(szErr, "PassportAuthFail") == 0 && allowRecurse) {
				MSN_GetPassportAuth();
				status = MSN_StoreCreateDocument(sztName, szMimeType, szPicData, false) ? 200 : 500;
			}
			ezxml_free(xmlm);
		}
	}

	mir_free(tResult);
	mir_free(storeUrl);

	return status == 200;
}


bool CMsnProto::MSN_StoreUpdateDocument(const TCHAR *sztName, const char *szMimeType, const char *szPicData, bool allowRecurse)
{
	char* reqHdr;
	char* szName = mir_utf8encodeT(sztName);
	ezxml_t tbdy;
	ezxml_t xmlp = storeSoapHdr("UpdateDocument", "RoamingIdentityChanged", tbdy, reqHdr);

	ezxml_t doc = ezxml_add_child(tbdy, "document", 0);
	ezxml_set_attr(doc, "xsi:type", "Photo");
	ezxml_t node = ezxml_add_child(doc, "ResourceID", 0);
	ezxml_set_txt(node, photoid);
	node = ezxml_add_child(doc, "Name", 0);
	ezxml_set_txt(node, szName);

	doc = ezxml_add_child(doc, "DocumentStreams", 0);
	doc = ezxml_add_child(doc, "DocumentStream", 0);
	ezxml_set_attr(doc, "xsi:type", "PhotoStream");

	node = ezxml_add_child(doc, "MimeType", 0);
	ezxml_set_txt(node, szMimeType);
	node = ezxml_add_child(doc, "Data", 0);
	ezxml_set_txt(node, szPicData);
	node = ezxml_add_child(doc, "DataSize", 0);
	ezxml_set_txt(node, "0");
	node = ezxml_add_child(doc, "DocumentStreamType", 0);
	ezxml_set_txt(node, "UserTileStatic");

	char* szData = ezxml_toxml(xmlp, true);

	ezxml_free(xmlp);
	mir_free(szName);

	unsigned status = 0;
	char *storeUrl = NULL, *tResult = NULL;

	for (int k = 4; --k;) {
		mir_free(storeUrl);
		storeUrl = GetStoreHost("UpdateDocument");
		tResult = getSslResult(&storeUrl, szData, reqHdr, status);
		if (tResult == NULL) UpdateStoreHost("UpdateDocument", NULL);
		else break;
	}

	mir_free(reqHdr);
	free(szData);

	if (tResult != NULL) {
		UpdateStoreHost("UpdateDocument", storeUrl);
		if (status == 500 && allowRecurse) {
			ezxml_t xmlm = ezxml_parse_str(tResult, strlen(tResult));
			const char* szErr = ezxml_txt(getSoapFault(xmlm, true));
			if (strcmp(szErr, "PassportAuthFail") == 0) {
				MSN_GetPassportAuth();
				status = MSN_StoreUpdateDocument(sztName, szMimeType, szPicData, false) ? 200 : 500;
			}
			else if (szErr[0]) {
				MSN_StoreDeleteRelationships(true);
				MSN_StoreDeleteRelationships(false);

				MSN_StoreCreateDocument(sztName, szMimeType, szPicData);
				MSN_StoreCreateRelationships();
			}
			ezxml_free(xmlm);
		}
	}

	mir_free(tResult);
	mir_free(storeUrl);

	return status == 200;
}

bool CMsnProto::MSN_StoreFindDocuments(bool allowRecurse)
{
	char* reqHdr;
	ezxml_t tbdy;
	ezxml_t xmlp = storeSoapHdr("FindDocuments", "RoamingIdentityChanged", tbdy, reqHdr);

	ezxml_t srch = ezxml_add_child(tbdy, "objectHandle", 0);
	ezxml_t node = ezxml_add_child(srch, "RelationshipName", 0);
	ezxml_set_txt(node, "/UserTiles");

	ezxml_t alias = ezxml_add_child(srch, "Alias", 0);
	node = ezxml_add_child(alias, "Name", 0);
	ezxml_set_txt(node, mycid);
	node = ezxml_add_child(alias, "NameSpace", 0);
	ezxml_set_txt(node, "MyCidStuff");

	ezxml_t doc = ezxml_add_child(tbdy, "documentAttributes", 0);
	node = ezxml_add_child(doc, "ResourceID", 0);
	ezxml_set_txt(node, "true");
	node = ezxml_add_child(doc, "Name", 0);
	ezxml_set_txt(node, "true");

	doc = ezxml_add_child(tbdy, "documentFilter", 0);
	node = ezxml_add_child(doc, "FilterAttributes", 0);
	ezxml_set_txt(node, "None");

	doc = ezxml_add_child(tbdy, "documentSort", 0);
	node = ezxml_add_child(doc, "SortBy", 0);
	ezxml_set_txt(node, "DateModified");

	doc = ezxml_add_child(tbdy, "findContext", 0);
	node = ezxml_add_child(doc, "FindMethod", 0);
	ezxml_set_txt(node, "Default");
	node = ezxml_add_child(doc, "ChunkSize", 0);
	ezxml_set_txt(node, "25");

	char* szData = ezxml_toxml(xmlp, true);

	ezxml_free(xmlp);

	unsigned status = 0;
	char *storeUrl = NULL, *tResult = NULL;

	for (int k = 4; --k;) {
		mir_free(storeUrl);
		storeUrl = GetStoreHost("FindDocuments");
		tResult = getSslResult(&storeUrl, szData, reqHdr, status);
		if (tResult == NULL) UpdateStoreHost("FindDocuments", NULL);
		else break;
	}

	mir_free(reqHdr);
	free(szData);

	if (tResult != NULL) {
		UpdateStoreHost("FindDocuments", storeUrl);
		if (status == 500) {
			ezxml_t xmlm = ezxml_parse_str(tResult, strlen(tResult));
			const char* szErr = ezxml_txt(getSoapFault(xmlm, true));
			if (strcmp(szErr, "PassportAuthFail") == 0 && allowRecurse) {
				MSN_GetPassportAuth();
				status = MSN_StoreFindDocuments(false) ? 200 : 500;
			}
			ezxml_free(xmlm);
		}
	}

	mir_free(tResult);
	mir_free(storeUrl);

	return status == 200;
}
