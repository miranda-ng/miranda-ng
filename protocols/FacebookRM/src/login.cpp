/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2011-17 Robert Pösel, 2017-19 Miranda NG team

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

HttpRequest* facebook_client::loginRequest()
{
	HttpRequest *p = new HttpRequest(REQUEST_POST, FACEBOOK_SERVER_LOGIN "/login.php");
	p->flags |= NLHRF_NODUMP;
	p << INT_PARAM("login_attempt", 1) << INT_PARAM("lwv", 110);
	return p;
}

HttpRequest* facebook_client::loginRequest(const char *username, const char *password, const char *urlData, const char *bodyData)
{
	HttpRequest *p = new HttpRequest(REQUEST_POST, FACEBOOK_SERVER_LOGIN "/login.php");
	p->flags |= NLHRF_NODUMP;
	p->Persistent = p->NONE;

	p << INT_PARAM("login_attempt", 1) << INT_PARAM("lwv", 110);
	if (mir_strlen(urlData)) {
		p->m_szParam.AppendChar('&');
		p->m_szParam.Append(urlData); // additional data parsed from form
	}
		
	p->Body
		<< INT_PARAM("persistent", 1)
		<< CHAR_PARAM("email", username)
		<< CHAR_PARAM("pass", password)
		<< CHAR_PARAM("lgndim", "eyJ3IjoxOTIwLCJoIjoxMDgwLCJhdyI6MTgzNCwiYWgiOjEwODAsImMiOjMyfQ==") // means base64 encoded: {"w":1920,"h":1080,"aw":1834,"ah":1080,"c":32}
		<< bodyData; // additional data parsed from form

	return p;
}

//////////////////////////////////////////////////////////////////////////////////////////
// request to receive login code via SMS

HttpRequest* facebook_client::loginSmsRequest(const char *dtsg)
{
	HttpRequest *p = new HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/ajax/login/approvals/send_sms");
		
	p << INT_PARAM("dpr", 1);

	p->Body
		<< CHAR_PARAM("method_requested", "sms_requested")
		<< INT_PARAM("__a", 1)
		<< INT_PARAM("__user", 0)
		<< INT_PARAM("__be", 0)
		<< CHAR_PARAM("__pc", "EXP1:DEFAULT")
		<< CHAR_PARAM("current_time", (utils::time::unix_timestamp() + ".000").c_str())
		<< CHAR_PARAM("__dyn", __dyn())
		<< CHAR_PARAM("__req", __req())
		<< CHAR_PARAM("fb_dtsg", dtsg)
		<< CHAR_PARAM("ttstamp", ttstamp_.c_str())
		<< CHAR_PARAM("__rev", __rev());

	return p;
}

//////////////////////////////////////////////////////////////////////////////////////////
// setting machine name

HttpRequest* facebook_client::setupMachineRequest()
{
	HttpRequest *p = new HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/checkpoint/");
	p->m_szParam.Append("next");
	return p;
}

HttpRequest* facebook_client::setupMachineRequest(const char *dtsg, const char *nh, const char *submit)
{
	HttpRequest *p = new HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/checkpoint/");

	p->m_szParam.Append("next");

	p->Body
		<< CHAR_PARAM(CMStringA(::FORMAT, "submit[%s]", submit), submit)
		<< CHAR_PARAM("nh", nh)
		<< CHAR_PARAM("fb_dtsg", dtsg);

	return p;
}

//////////////////////////////////////////////////////////////////////////////////////////
// disconnecting physically

HttpRequest* facebook_client::logoutRequest()
{
	HttpRequest *p = new HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/logout.php");

	p->flags |= NLHRF_REDIRECT | NLHRF_NODUMP;

	p->Body
		<< CHAR_PARAM("fb_dtsg", dtsg_.c_str())
		<< CHAR_PARAM("h", logout_hash_.c_str());
	
	return p;
}
