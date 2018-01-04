/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2011-17 Robert Pösel, 2017-18 Miranda NG team

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

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////////////////////////
// connecting physically

LoginRequest::LoginRequest() :
	HttpRequest(REQUEST_POST, FACEBOOK_SERVER_LOGIN "/login.php")
{
	Url << INT_PARAM("login_attempt", 1);
}

LoginRequest::LoginRequest(const char *username, const char *password, const char *urlData, const char *bodyData) :
	HttpRequest(REQUEST_POST, FACEBOOK_SERVER_LOGIN "/login.php")
{
	Persistent = NONE;

	Url
		<< INT_PARAM("login_attempt", 1)
		<< urlData; // additional data parsed from form
		
	Body
		<< INT_PARAM("persistent", 1)
		<< CHAR_PARAM("email", ptrA(mir_urlEncode(username)))
		<< CHAR_PARAM("pass", ptrA(mir_urlEncode(password)))
		<< CHAR_PARAM("lgndim", "eyJ3IjoxOTIwLCJoIjoxMDgwLCJhdyI6MTgzNCwiYWgiOjEwODAsImMiOjMyfQ==") // means base64 encoded: {"w":1920,"h":1080,"aw":1834,"ah":1080,"c":32}
		<< bodyData; // additional data parsed from form
}

//////////////////////////////////////////////////////////////////////////////////////////
// request to receive login code via SMS

LoginSmsRequest::LoginSmsRequest(facebook_client *fc, const char *dtsg) :
	HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/ajax/login/approvals/send_sms")
{
	Url << INT_PARAM("dpr", 1);

	Body
		<< CHAR_PARAM("method_requested", "sms_requested")
		<< INT_PARAM("__a", 1)
		<< INT_PARAM("__user", 0)
		<< INT_PARAM("__be", 0)
		<< CHAR_PARAM("__pc", "EXP1:DEFAULT")
		<< CHAR_PARAM("current_time", (utils::time::unix_timestamp() + ".000").c_str())
		<< CHAR_PARAM("__dyn", fc->__dyn())
		<< CHAR_PARAM("__req", fc->__req())
		<< CHAR_PARAM("fb_dtsg", dtsg)
		<< CHAR_PARAM("ttstamp", fc->ttstamp_.c_str())
		<< CHAR_PARAM("__rev", fc->__rev());
}

//////////////////////////////////////////////////////////////////////////////////////////
// setting machine name

SetupMachineRequest::SetupMachineRequest() :
	HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/checkpoint/")
{
	Url << "next";
}

SetupMachineRequest::SetupMachineRequest(const char *dtsg, const char *nh, const char *submit) :
	HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/checkpoint/")
{
	Url << "next";

	Body
		<< CHAR_PARAM(CMStringA(::FORMAT, "submit[%s]", submit), submit)
		<< CHAR_PARAM("nh", nh)
		<< CHAR_PARAM("fb_dtsg", dtsg);
}

//////////////////////////////////////////////////////////////////////////////////////////
// disconnecting physically

LogoutRequest::LogoutRequest(const char *dtsg, const char *logoutHash) :
	HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/logout.php")
{
	Body
		<< CHAR_PARAM("fb_dtsg", dtsg)
		<< CHAR_PARAM("h", logoutHash);
}
