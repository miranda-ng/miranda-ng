/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2011-17 Robert Pösel

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

#ifndef _FACEBOOK_REQUEST_LOGIN_H_
#define _FACEBOOK_REQUEST_LOGIN_H_

// connecting physically
class LoginRequest : public HttpRequest
{
public:
	LoginRequest() :
		HttpRequest(REQUEST_POST, FACEBOOK_SERVER_LOGIN "/login.php")
	{
		Url
			<< "login_attempt=1";
	}

	LoginRequest(const char *username, const char *password, const char *urlData, const char *bodyData) :
		HttpRequest(REQUEST_POST, FACEBOOK_SERVER_LOGIN "/login.php")
	{
		Persistent = NONE;

		Url
			<< "login_attempt=1"
			<< urlData; // additional data parsed from form
		
		Body
			<< "persistent=1"
			<< CHAR_VALUE("email", ptrA(mir_urlEncode(username)))
			<< CHAR_VALUE("pass", ptrA(mir_urlEncode(password)))
			<< CHAR_VALUE("lgndim", "eyJ3IjoxOTIwLCJoIjoxMDgwLCJhdyI6MTgzNCwiYWgiOjEwODAsImMiOjMyfQ==") // means base64 encoded: {"w":1920,"h":1080,"aw":1834,"ah":1080,"c":32}
			<< bodyData; // additional data parsed from form
	}
};

// request to receive login code via SMS
class LoginSmsRequest : public HttpRequest
{
public:
	LoginSmsRequest(facebook_client *fc, const char *dtsg) :
		HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/ajax/login/approvals/send_sms")
	{
		Url
			<< "dpr=1";

		Body
			<< "method_requested=sms_requested"
			<< "__a=1"
			<< "__user=0"
			<< "__be=0"
			<< "__pc=EXP1:DEFAULT"
			<< CHAR_VALUE("current_time", (utils::time::unix_timestamp() + ".000").c_str())
			<< CHAR_VALUE("__dyn", fc->__dyn())
			<< CHAR_VALUE("__req", fc->__req())
			<< CHAR_VALUE("fb_dtsg", dtsg)
			<< CHAR_VALUE("ttstamp", fc->ttstamp_.c_str())
			<< CHAR_VALUE("__rev", fc->__rev());
	}
};

// setting machine name
class SetupMachineRequest : public HttpRequest
{
public:
	SetupMachineRequest() :
		HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/checkpoint/")
	{
		Url
			<< "next";
	}

	SetupMachineRequest(const char *dtsg, const char *nh, const char *submit) :
		HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/checkpoint/")
	{
		Url
			<< "next";

		Body
			<< CMStringA(::FORMAT, "submit[%s]=%s", submit, submit).c_str()
			<< CHAR_VALUE("nh", nh)
			<< CHAR_VALUE("fb_dtsg", dtsg);
	}
};

// disconnecting physically
class LogoutRequest : public HttpRequest
{
public:
	LogoutRequest(const char *dtsg, const char *logoutHash) :
		HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/logout.php")
	{
		Body
			<< "ref="
			<< CHAR_VALUE("fb_dtsg", dtsg)
			<< CHAR_VALUE("h", logoutHash);
	}
};

#endif //_FACEBOOK_REQUEST_LOGIN_H_
