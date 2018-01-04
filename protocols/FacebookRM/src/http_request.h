/*
Copyright (c) 2015-18 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_

class HttpRequest : public NETLIBHTTPREQUEST, public MZeroedObject
{
	HttpRequest& operator=(const HttpRequest&); // to prevent copying;

	va_list formatArgs;
	CMStringA url;

protected:
	enum HttpRequestUrlFormat { FORMAT };

	class HttpRequestUrl 
	{
		friend HttpRequest;

	private:
		HttpRequest & request;

		HttpRequestUrl(HttpRequest &request, const char *url) : request(request)
		{
			request.url = url;
			request.szUrl = request.url.GetBuffer();
		}

		HttpRequestUrl(HttpRequest &request, const char *urlFormat, va_list args) : request(request)
		{
			request.url.AppendFormatV(urlFormat, args);
			request.szUrl = request.url.GetBuffer();
		}

		HttpRequestUrl& operator=(const HttpRequestUrl&); // to prevent copying;

	public:
		HttpRequestUrl& operator<<(const char *param)
		{
			if (param)
				request.AddUrlParameter("%s", param);
			return *this;
		}

		HttpRequestUrl& operator<<(const BOOL_PARAM &param)
		{
			request.AddUrlParameter("%s=%s", param.szName, param.bValue ? "true" : "false");
			return *this;
		}

		HttpRequestUrl& operator<<(const INT_PARAM &param)
		{
			request.AddUrlParameter("%s=%i", param.szName, param.iValue);
			return *this;
		}

		HttpRequestUrl& operator<<(const INT64_PARAM &param)
		{
			request.AddUrlParameter("%s=%lld", param.szName, param.iValue);
			return *this;
		}

		HttpRequestUrl& operator<<(const CHAR_PARAM &param)
		{
			request.AddUrlParameter("%s=%s", param.szName, param.szValue);
			return *this;
		}

		char* ToString()
		{
			return request.url.GetBuffer();
		}
	};

	class HttpRequestHeaders
	{
		HttpRequestHeaders& operator=(const HttpRequestHeaders&); // to prevent copying;

		HttpRequest &request;

		void Add(LPCSTR szName)
		{
			Add(szName, "");
		}

		void Add(LPCSTR szName, LPCSTR szValue)
		{
			request.headers = (NETLIBHTTPHEADER*)mir_realloc(
				request.headers, sizeof(NETLIBHTTPHEADER)* (request.headersCount + 1));
			request.headers[request.headersCount].szName = mir_strdup(szName);
			request.headers[request.headersCount].szValue = mir_strdup(szValue);
			request.headersCount++;
		}

	public:
		HttpRequestHeaders(HttpRequest &request) : request(request) {}

		HttpRequestHeaders& operator<<(const CHAR_PARAM &param)
		{
			Add(param.szName, param.szValue);
			return *this;
		}
	};

	class HttpRequestBody
	{
	private:
		CMStringA content;

		void AppendSeparator()
		{
			if (!content.IsEmpty())
				content.AppendChar('&');
		}

	public:
		HttpRequestBody() {}

		HttpRequestBody& operator<<(const char *str)
		{
			AppendSeparator();
			if (str != nullptr)
				content.Append(str);
			return *this;
		}

		HttpRequestBody& operator<<(const BOOL_PARAM &param)
		{
			AppendSeparator();
			content.AppendFormat("%s=%s", param.szName, param.bValue ? "true" : "false");
			return *this;
		}

		HttpRequestBody& operator<<(const INT_PARAM &param)
		{
			AppendSeparator();
			content.AppendFormat("%s=%i", param.szName, param.iValue);
			return *this;
		}

		HttpRequestBody& operator<<(const INT64_PARAM &param)
		{
			AppendSeparator();
			content.AppendFormat("%s=%lld", param.szName, param.iValue);
			return *this;
		}

		HttpRequestBody& operator<<(const CHAR_PARAM &param)
		{
			AppendSeparator();
			content.AppendFormat("%s=%s", param.szName, param.szValue);
			return *this;
		}

		char* ToString()
		{
			return content.GetBuffer();
		}
	};

	void AddUrlParameter(const char *fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		url += (url.Find('?') == -1) ? '?' : '&';
		url.AppendFormatV(fmt, args);
		va_end(args);
		szUrl = url.GetBuffer();
	}

public:
	HttpRequestUrl Url;
	HttpRequestHeaders Headers;
	HttpRequestBody Body;

	enum PersistentType { NONE, DEFAULT, CHANNEL, MESSAGES };

	bool NotifyErrors;
	PersistentType Persistent;

	HttpRequest(int type, LPCSTR url)
		: Url(*this, url), Headers(*this)
	{
		cbSize = sizeof(NETLIBHTTPREQUEST);
		flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_DUMPASTEXT;
		requestType = type;
		pData = nullptr;
		timeout = 20 * 1000;

		NotifyErrors = true;
		Persistent = DEFAULT;
	}

	HttpRequest(int type, HttpRequestUrlFormat, LPCSTR urlFormat, ...)
		: Url(*this, urlFormat, (va_start(formatArgs, urlFormat), formatArgs)), Headers(*this)
	{
		cbSize = sizeof(NETLIBHTTPREQUEST);
		flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_DUMPASTEXT;
		requestType = type;
		va_end(formatArgs);
		pData = nullptr;
		timeout = 20 * 1000;

		NotifyErrors = true;
		Persistent = DEFAULT;
	}

	virtual ~HttpRequest()
	{
		for (int i = 0; i < headersCount; i++) {
			mir_free(headers[i].szName);
			mir_free(headers[i].szValue);
		}
		mir_free(headers);
	}

	virtual NETLIBHTTPREQUEST* Send(HNETLIBUSER nlu)
	{
		if (url.Find("://") == -1)
			url.Insert(0, ((flags & NLHRF_SSL) ? "https://" : "http://"));
		szUrl = url.GetBuffer();

		if (!pData) {
			pData = Body.ToString();
			dataLength = (int)mir_strlen(pData);
		}

		Netlib_Logf(nlu, "Send request to %s", szUrl);

		return Netlib_HttpTransaction(nlu, this);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// channel.cpp

struct ChannelRequest : public HttpRequest
{
	enum Type { PULL, PING };

	ChannelRequest(facebook_client *fc, Type type);
};

//////////////////////////////////////////////////////////////////////////////////////////
// contact.cpp

// getting frienship requests (using mobile website)
struct GetFriendshipsRequest : public HttpRequest
{
	GetFriendshipsRequest(bool mobileBasicWorks) :
		HttpRequest(REQUEST_GET, FORMAT, "%s/friends/center/requests/", mobileBasicWorks ? FACEBOOK_SERVER_MBASIC : FACEBOOK_SERVER_MOBILE)
	{
		flags |= NLHRF_REDIRECT;
	}
};

// getting info about particular friend
struct UserInfoRequest : public HttpRequest
{	UserInfoRequest(facebook_client *fc, const LIST<char> &userIds);
};

// getting info about all friends
struct UserInfoAllRequest : public HttpRequest
{	UserInfoAllRequest(facebook_client *fc);
};

// requesting friendships
struct AddFriendRequest : public HttpRequest
{	AddFriendRequest(facebook_client *fc, const char *userId);
};

// deleting friendships
struct DeleteFriendRequest : public HttpRequest
{	DeleteFriendRequest(facebook_client *fc, const char *userId);
};

// canceling (our) friendship request
struct CancelFriendshipRequest : public HttpRequest
{	CancelFriendshipRequest(facebook_client *fc, const char *userId);
};

// approving or ignoring friendship requests
struct AnswerFriendshipRequest : public HttpRequest
{
	enum Answer { CONFIRM, REJECT };

	AnswerFriendshipRequest(facebook_client *fc, const char *userId, Answer answer);
};

//////////////////////////////////////////////////////////////////////////////////////////
// feeds.cpp

// getting newsfeed posts
struct NewsfeedRequest : public HttpRequest
{	NewsfeedRequest(facebook_client *fc);
};

// getting memories ("on this day") posts
struct MemoriesRequest : public HttpRequest
{	MemoriesRequest(facebook_client *fc);
};

//////////////////////////////////////////////////////////////////////////////////////////
// history.cpp

// getting thread info and messages
class ThreadInfoRequest : public HttpRequest
{
	void setCommonBody(facebook_client *fc);

public:
	// Request only messages history
	ThreadInfoRequest(facebook_client *fc, bool isChat, const char *id, const char* timestamp = nullptr, int limit = -1);
	ThreadInfoRequest(facebook_client *fc, const LIST<char> &ids, int offset, int limit);
};

// getting unread threads
struct UnreadThreadsRequest : public HttpRequest
{	UnreadThreadsRequest(facebook_client *fc);
};

//////////////////////////////////////////////////////////////////////////////////////////
// login.cpp

// connecting physically
struct LoginRequest : public HttpRequest
{	
	LoginRequest();
	LoginRequest(const char *username, const char *password, const char *urlData, const char *bodyData);
};

// disconnecting physically
struct LogoutRequest : public HttpRequest
{	LogoutRequest(const char *dtsg, const char *logoutHash);
};

// request to receive login code via SMS
struct LoginSmsRequest : public HttpRequest
{	LoginSmsRequest(facebook_client *fc, const char *dtsg);
};

// setting machine name
struct SetupMachineRequest : public HttpRequest
{	
	SetupMachineRequest();
	SetupMachineRequest(const char *dtsg, const char *nh, const char *submit);
};

//////////////////////////////////////////////////////////////////////////////////////////
// messages.cpp

// sending messages
struct SendMessageRequest : public HttpRequest
{	SendMessageRequest(facebook_client *fc, const char *userId, const char *threadId, const char *messageId, const char *messageText, bool isChat, const char *captcha, const char *captchaPersistData);
};

// sending typing notification
struct SendTypingRequest : public HttpRequest
{	SendTypingRequest(facebook_client *fc, const char *userId, bool isChat, bool isTyping);
};

struct MarkMessageReadRequest : public HttpRequest
{	MarkMessageReadRequest(facebook_client *fc, const LIST<char> &ids);
};

//////////////////////////////////////////////////////////////////////////////////////////
// notifications.cpp

// getting notifications
struct GetNotificationsRequest : public HttpRequest
{	GetNotificationsRequest(facebook_client *fc, int count);
};

// marking notifications read
struct MarkNotificationReadRequest : public HttpRequest
{	MarkNotificationReadRequest(facebook_client *fc, const char *id);
};

//////////////////////////////////////////////////////////////////////////////////////////
// profile.cpp

// getting own name, avatar, ...
struct HomeRequest : public HttpRequest
{	HomeRequest();
};

// getting fb_dtsg
struct DtsgRequest : public HttpRequest
{	DtsgRequest();
};

// request mobile page containing profile picture
struct ProfilePictureRequest : public HttpRequest
{	ProfilePictureRequest(bool mobileBasicWorks, const char *userId);
};

// request mobile page containing user profile
struct ProfileRequest : public HttpRequest
{	ProfileRequest(bool mobileBasicWorks, const char *data);
};

// request mobile page containing user profile by his id, and in english language (for parsing data)
struct ProfileInfoRequest : public HttpRequest
{	ProfileInfoRequest(bool mobileBasicWorks, const char *userId);
};

//////////////////////////////////////////////////////////////////////////////////////////
// search.cpp

// searching
struct SearchRequest : public HttpRequest
{	SearchRequest(bool mobileBasicWorks, const char *query, int s, int pn, const char *ssid);
};

//////////////////////////////////////////////////////////////////////////////////////////
// status.cpp

// getting info about channel and connecting to it
struct ReconnectRequest : public HttpRequest
{	ReconnectRequest(facebook_client *fc);
};

// setting chat visibility
struct SetVisibilityRequest : public HttpRequest
{	SetVisibilityRequest(facebook_client *fc, bool online);
};

//////////////////////////////////////////////////////////////////////////////////////////
// utils.cpp

// getting data for given url (for sending/posting reasons)
struct LinkScraperRequest : public HttpRequest
{	LinkScraperRequest(facebook_client *fc, status_data *status);
};

// refreshing captcha dialog (changing captcha type)
struct RefreshCaptchaRequest : public HttpRequest
{	RefreshCaptchaRequest(facebook_client *fc, const char *captchaPersistData);
};

// getting owned/admined pages list
struct GetPagesRequest : public HttpRequest
{
	GetPagesRequest() :
		HttpRequest(REQUEST_GET, FACEBOOK_SERVER_REGULAR "/bookmarks/pages")
	{}
};

// changing identity to post status for pages
struct SwitchIdentityRequest : public HttpRequest
{	SwitchIdentityRequest(const char *dtsg, const char *userId);
};

// posting status to our or friends's wall
struct SharePostRequest : public HttpRequest
{	SharePostRequest(facebook_client *fc, status_data *status, const char *linkData);
};

// sending pokes
struct SendPokeRequest : public HttpRequest
{	SendPokeRequest(facebook_client *fc, const char *userId);
};

#endif //_HTTP_REQUEST_H_