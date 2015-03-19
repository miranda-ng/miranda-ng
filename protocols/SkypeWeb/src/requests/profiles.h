#ifndef _SKYPE_REQUEST_PROFILES_H_
#define _SKYPE_REQUEST_PROFILES_H_

class GetProfilesRequest : public HttpRequest
{
public:
	GetProfilesRequest(const char *token, const LIST<char> &skypenames) :
		HttpRequest(REQUEST_POST, "api.skype.com//users/self/contacts/profiles")
	{
		flags |= NLHRF_SSL;

		CMStringA data;
		for (size_t i = 0; i < skypenames.getCount(); i++)
		{
			data.AppendFormat("contacts[]=%s&", skypenames[i]);
		}
		data.Delete(data.GetLength() - 1);

		SetData(data, data.GetLength());

		AddHeader("X-Skypetoken", token);
		AddHeader("Accept", "application/json");
		AddHeader("Content-Type", "application/x-www-form-urlencoded");
	}
};

#endif //_SKYPE_REQUEST_PROFILES_H_
