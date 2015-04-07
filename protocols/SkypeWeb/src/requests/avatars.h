#ifndef _SKYPE_REQUEST_AVATAR_H_
#define _SKYPE_REQUEST_AVATAR_H_

class GetAvatarRequest : public HttpRequest
{
public:
	GetAvatarRequest(const char *url) : HttpRequest(REQUEST_GET, url)
	{
		flags |= NLHRF_REDIRECT;
	}
};

class SetAvatarRequest : public HttpRequest
{
public:
	SetAvatarRequest(const char *token, const char *skypename, const char *data, size_t dataSize) :
		HttpRequest(REQUEST_PUT, FORMAT, "api.skype.com/users/%s/profile/avatar", skypename)
	{
		Headers
			<< CHAR_VALUE("X-Skypetoken", token)
			<< CHAR_VALUE("Content-Type", "image/jpeg");

		pData = (char*)mir_alloc(dataSize);
		memcpy(pData, data, dataSize);
		dataLength = dataSize;
	}

	~SetAvatarRequest()
	{
		mir_free(pData);
	}
};


#endif //_SKYPE_REQUEST_AVATAR_H_
