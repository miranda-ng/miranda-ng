/*
Copyright (c) 2013-22 Miranda NG team (https://miranda-ng.org)

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

#include "stdafx.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////////////////
// Captcha form


bool CVkProto::RunCaptchaForm(LPCSTR szUrl, CMStringA &result)
{
	debugLogA("CVkProto::RunCaptchaForm: reading picture from %s", szUrl);
	result.Empty();

	CAPTCHA_FORM_PARAMS param = {};
	if (getBool("UseCaptchaAssistant", false)) {
		CMStringA szCaptchaAssistant(FORMAT, "http://ca.tiflohelp.ru/?link=%s", ptrA(ExpUrlEncode(szUrl)));
		Utils_OpenUrl(szCaptchaAssistant);
	}
	else {
		NETLIBHTTPREQUEST req = { sizeof(req) };
		req.requestType = REQUEST_GET;
		req.szUrl = (LPSTR)szUrl;
		req.flags = VK_NODUMPHEADERS;

		NETLIBHTTPREQUEST *reply = Netlib_HttpTransaction(m_hNetlibUser, &req);
		if (reply == nullptr)
			return false;

		if (reply->resultCode != 200) {
			debugLogA("CVkProto::RunCaptchaForm: failed with code %d", reply->resultCode);
			return false;
		}

		param.bmp = Image_LoadFromMem(reply->pData, reply->dataLength, FIF_UNKNOWN);

		BITMAP bmp = { 0 };
		GetObject(param.bmp, sizeof(bmp), &bmp);
		param.w = bmp.bmWidth;
		param.h = bmp.bmHeight;
	}

	CVkCaptchaForm dlg(this, &param);
	if (!dlg.DoModal())
		return false;

	debugLogA("CVkProto::RunCaptchaForm: user entered text %s", param.Result);
	result = param.Result;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// fill a request from JSON

bool CVkProto::ApplyCaptcha(AsyncHttpRequest *pReq, const JSONNode &jnErrorNode)
{
	debugLogA("CVkProto::ApplyCaptcha");
	if (!IsOnline())
		return false;

	CMStringA szUrl(jnErrorNode["captcha_img"].as_mstring());
	CMStringA szSid(jnErrorNode["captcha_sid"].as_mstring());

	if (szUrl.IsEmpty() || szSid.IsEmpty())
		return false;

	CMStringA userReply;
	if (!RunCaptchaForm(szUrl, userReply))
		return false;

	pReq << CHAR_PARAM("captcha_sid", szSid) << CHAR_PARAM("captcha_key", userReply.GetString());
	pReq->bNeedsRestart = true;
	return true;
}