#ifndef _SKYPE_REQUEST_LOGIN_H_
#define _SKYPE_REQUEST_LOGIN_H_

class LoginRequest : public HttpRequest
{
public:
	LoginRequest() :
		HttpRequest(REQUEST_POST, "login.skype.com/login")
	{
		flags |= NLHRF_SSL;

		url.Append("?client_id=578134&redirect_uri=https%3A%2F%2Fweb.skype.com");
	}

	LoginRequest(const char *skypename, const char *password, const char *pie, const char *etm) :
		HttpRequest(REQUEST_POST, "login.skype.com/login")
	{
		flags |= NLHRF_SSL;

		url.Append("?client_id=578134&redirect_uri=https%3A%2F%2Fweb.skype.com");

		LPTIME_ZONE_INFORMATION tzi = tmi.getTziByContact(NULL);

		char sign = tzi->Bias > 0 ? '-' : '+';
		int hours = tzi->Bias / -60;
		int minutes = tzi->Bias % -60;

		CMStringA data = "";
		data.AppendFormat("username=%s&", skypename);
		data.AppendFormat("password=%s&", password);
		data.AppendFormat("pie=%s&", ptrA(mir_urlEncode(pie)));
		data.AppendFormat("etm=%s&", ptrA(mir_urlEncode(etm)));
		data.AppendFormat("timezone_field=%c%02d|%02d&", sign, hours, minutes);
		data.AppendFormat("js_time=%d.00&", time(NULL));
		data.Append("client_id=578134&");
		data.Append("redirect_uri=https%3A%2F%2Fweb.skype.com");

		SetData(data, data.GetLength());

		AddHeader("Content-Type", "application/x-www-form-urlencoded");
	}
};

#endif //_SKYPE_REQUEST_LOGIN_H_
