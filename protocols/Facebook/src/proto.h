/*

Facebook plugin for Miranda NG
Copyright © 2019 Miranda NG team

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#pragma once


/**
 * FB_API_AHOST:
 *
 * The HTTP host for the Facebook API.
 */
#define FB_API_AHOST  "https://api.facebook.com"

/**
 * FB_API_BHOST:
 *
 * The HTTP host for the Facebook BAPI.
 */
#define FB_API_BHOST  "https://b-api.facebook.com"

/**
 * FB_API_GHOST:
 *
 * The HTTP host for the Facebook Graph API.
 */
#define FB_API_GHOST  "https://graph.facebook.com"

/**
 * FB_API_WHOST:
 *
 * The HTTP host for the Facebook website.
 */
#define FB_API_WHOST  "https://www.facebook.com"

/**
 * FB_API_FBRPC_PREFIX
 *
 * The fbrpc URL prefix used in links shared from the mobile app.
 */
#define FB_API_FBRPC_PREFIX "fbrpc://facebook/nativethirdparty"

/**
 * FB_API_KEY:
 *
 * The Facebook API key.
 */
#define FB_API_KEY  "256002347743983"

/**
 * FB_API_SECRET:
 *
 * The Facebook API secret.
 */
#define FB_API_SECRET  "374e60f8b9bb6b8cbb30f78030438895"

/**
 * FB_ORCA_AGENT
 *
 * The part of the user agent that looks like the official client, since the
 * server started checking this.
 */

#define FB_ORCA_AGENT "[FBAN/Orca-Android;FBAV/192.0.0.31.101;FBPN/com.facebook.orca;FBLC/en_US;FBBV/52182662]"

/**
 * FB_API_AGENT:
 *
 * The HTTP User-Agent header.
 */
#define FB_API_AGENT  "Facebook plugin / Purple / 0.9.6 " FB_ORCA_AGENT

/**
 * FB_API_MQTT_AGENT
 *
 * The client information string sent in the MQTT CONNECT message
 */

#define FB_API_MQTT_AGENT FB_API_AGENT

/**
 * FB_API_URL_ATTACH:
 *
 * The URL for attachment URL requests.
 */
#define FB_API_URL_ATTACH  FB_API_AHOST "/method/messaging.getAttachment"
//#define FB_API_URL_ATTACH  FB_API_AHOST "/method/messaging.attachmentRedirect"

/**
 * FB_API_URL_AUTH:
 *
 * The URL for authentication requests.
 */
#define FB_API_URL_AUTH  FB_API_BHOST "/method/auth.login"

/**
 * FB_API_URL_GQL:
 *
 * The URL for GraphQL requests.
 */
#define FB_API_URL_GQL  FB_API_GHOST "/graphql"

/**
 * FB_API_URL_MESSAGES:
 *
 * The URL for linking message threads.
 */
#define FB_API_URL_MESSAGES  FB_API_WHOST "/messages"

/**
 * FB_API_URL_PARTS:
 *
 * The URL for participant management requests.
 */
#define FB_API_URL_PARTS  FB_API_GHOST "/participants"

/**
 * FB_API_URL_THREADS:
 *
 * The URL for thread management requests.
 */
#define FB_API_URL_THREADS  FB_API_GHOST "/me/group_threads"

/**
 * FB_API_URL_TOPIC:
 *
 * The URL for thread topic requests.
 */
#define FB_API_URL_TOPIC  FB_API_AHOST "/method/messaging.setthreadname"

/**
 * FB_API_QUERY_CONTACT:
 *
 * The query hash for the `UsersQuery`.
 *
 * Key mapping:
 *   0: user_fbids
 *   1: include_full_user_info
 *   2: profile_pic_large_size
 *   3: profile_pic_medium_size
 *   4: profile_pic_small_size
 */
#define FB_API_QUERY_CONTACT  10153915107411729

/**
 * FB_API_QUERY_CONTACTS:
 *
 * The query hash for the `FetchContactsFullQuery`.
 *
 * Key mapping:
 *   0: profile_types
 *   1: limit
 *   2: big_img_size
 *   3: huge_img_size
 *   4: small_img_size
 */
#define FB_API_QUERY_CONTACTS  10154444360806729

/**
 * FB_API_QUERY_CONTACTS_AFTER:
 *
 * The query hash for the `FetchContactsFullWithAfterQuery`.
 *
 * Key mapping:
 *   0: profile_types
 *   1: after
 *   2: limit
 *   3: big_img_size
 *   4: huge_img_size
 *   5: small_img_size
 */
#define FB_API_QUERY_CONTACTS_AFTER  10154444360816729


/**
 * FB_API_QUERY_CONTACTS_DELTA:
 *
 * The query hash for the `FetchContactsDeltaQuery`.
 *
 * Key mapping:
 *   0: after
 *   1: profile_types
 *   2: limit
 *   3: big_img_size
 *   4: huge_img_size
 *   5: small_img_size
 */
#define FB_API_QUERY_CONTACTS_DELTA  10154444360801729

/**
 * FB_API_QUERY_STICKER:
 *
 * The query hash for the `FetchStickersWithPreviewsQuery`.
 *
 * Key mapping:
 *   0: sticker_ids
 *   1: media_type
 *   2: preview_size
 *   3: scaling_factor
 *   4: animated_media_type
 */
#define FB_API_QUERY_STICKER  10152877994321729

/**
 * FB_API_QUERY_THREAD:
 *
 * The query hash for the `ThreadQuery`.
 *
 * Key mapping:
 *   0: thread_ids
 *   1: verification_type
 *   2: hash_key
 *   3: small_preview_size
 *   4: large_preview_size
 *   5: item_count
 *   6: event_count
 *   7: full_screen_height
 *   8: full_screen_width
 *   9: medium_preview_size
 *   10: fetch_users_separately
 *   11: include_message_info
 *   12: msg_count
 *   13: include_full_user_info
 *   14: profile_pic_large_size
 *   15: profile_pic_medium_size
 *   16: profile_pic_small_size
 */
#define FB_API_QUERY_THREAD  10153919752036729

/**
 * FB_API_QUERY_THREADS:
 *
 * The query hash for the `ThreadListQuery`.
 *
 * Key mapping:
 *   0: folder_tag
 *   1: thread_count
 *   2: include_thread_info
 *   3: verification_type
 *   4: hash_key
 *   5: small_preview_size
 *   6: large_preview_size
 *   7: item_count
 *   8: event_count
 *   9: full_screen_height
 *   10: full_screen_width
 *   11: medium_preview_size
 *   12: fetch_users_separately
 *   13: include_message_info
 *   14: msg_count
 *   15: UNKNOWN
 *   16: profile_pic_large_size
 *   17: profile_pic_medium_size
 *   18: profile_pic_small_size
 */
#define FB_API_QUERY_THREADS  10153919752026729

/**
 * FB_API_QUERY_SEQ_ID:
 *
 * A variant of ThreadListQuery with sequence ID
 *
 * TODO: parameters.
 */

#define FB_API_QUERY_SEQ_ID  10155268192741729

/**
 * FB_API_QUERY_XMA:
 *
 * The query hash for the `XMAQuery`.
 *
 * Key mapping:
 *   0: xma_id
 */
#define FB_API_QUERY_XMA  10153919431161729

/**
 * FB_API_CONTACTS_COUNT:
 *
 * The maximum amount of contacts to fetch in a single request. If this
 * value is set too high, HTTP request will fail. This is due to the
 * request data being too large.
 */
#define FB_API_CONTACTS_COUNT  "500"

#define FACEBOOK_MESSAGE_LIMIT 100000

class FacebookProto;

struct AsyncHttpRequest : public MTHttpRequest<FacebookProto>
{
	struct Param
	{
		Param(const char *p1, const char *p2) :
			key(p1), val(p2)
		{}
		
		CMStringA key, val;
	};
	OBJLIST<Param> params;

	AsyncHttpRequest();

	void CalcSig();
};

AsyncHttpRequest *operator<<(AsyncHttpRequest *, const CHAR_PARAM &);
AsyncHttpRequest *operator<<(AsyncHttpRequest *, const INT_PARAM &);

class JsonReply
{
	JSONNode *m_root = nullptr;
	int m_errorCode = 0;

public:
	JsonReply(NETLIBHTTPREQUEST *);
	~JsonReply();

	__forceinline JSONNode &data() const { return *m_root; }
	__forceinline int error() const { return m_errorCode; }
};

class FacebookProto : public PROTO<FacebookProto>
{
   uint8_t* doZip(size_t cbData, const void *pData, size_t &cbRes);

	AsyncHttpRequest *CreateRequest(const char *szName, const char *szMethod);
	AsyncHttpRequest *CreateRequestGQL(int64_t id);
	NETLIBHTTPREQUEST *ExecuteRequest(AsyncHttpRequest *pReq);

	// MQTT functions
	bool MqttConnect();
   void MqttOpen();
   void MqttPublish(const char *topic, const char *value);
   void MqttSubscribe(const char *topic, ...);
   void MqttUnsubscribe(const char *topic, ...);
   void MqttSend(const MqttMessage &payload);

	HNETLIBCONN m_mqttConn;

	// internal data
	CMStringA m_szDeviceID; // stored, GUID that identifies this miranda's account
	CMStringA m_szClientID; // stored, random alphanumeric string of 20 chars
	__int64   m_uid;        // stored, Facebook user id
	__int64   m_iMqttId;
   int16_t   m_mid;

	bool      m_invisible;
	bool      m_bOnline;

	CMStringA m_szAuthToken; // calculated 

   void OnLoggedIn();
   void OnLoggedOut();

	void __cdecl ServerThread(void *);

public:
	FacebookProto(const char *proto_name, const wchar_t *username);
	~FacebookProto();

    inline const char* ModuleName() const {
        return m_szModuleName;
    }

	////////////////////////////////////////////////////////////////////////////////////////
	// PROTO_INTERFACE

	void OnModulesLoaded() override;
   void OnShutdown() override;

	INT_PTR  GetCaps(int type, MCONTACT hContact) override;

	int      SetStatus(int iNewStatus) override;

    ////////////////////////

    // Services
    INT_PTR __cdecl SvcCreateAccMgrUI(WPARAM, LPARAM);
};

struct CMPlugin : public ACCPROTOPLUGIN<FacebookProto>
{
	CMPlugin();

	int Load() override;
};
