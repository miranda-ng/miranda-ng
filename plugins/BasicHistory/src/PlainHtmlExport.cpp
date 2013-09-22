/*
Basic History plugin
Copyright (C) 2011-2012 Krzysztof Kral

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

#include "StdAfx.h"
#include "PlainHtmlExport.h"
#include "Options.h"
#define EXP_FILE (*stream)

PlainHtmlExport::~PlainHtmlExport()
{
}

extern std::wstring MakeTextHtmled(const std::wstring& message, std::queue<std::pair<size_t, size_t> >* positionMap = NULL);
extern std::wstring UrlHighlightHtml(const std::wstring& message, bool& isUrl);

void PlainHtmlExport::WriteHeader(const std::wstring &fileName, const std::wstring &filterName, const std::wstring &myName, const std::wstring &myId, const std::wstring &name1, const std::wstring &proto1, const std::wstring &id1, const std::string& baseProto1, const std::wstring& encoding)
{
	EXP_FILE << _T("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n");
	EXP_FILE << _T("<html><head>\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=") << encoding << _T("\">\n");
	EXP_FILE << _T("<title>") << TranslateT("History Log") << _T(" [") << MakeTextHtmled(myName) << _T("] - [") << MakeTextHtmled(name1) << _T("]</title>\n");
	EXP_FILE << _T("<style type=\"text/css\"><!--\n");
	EXP_FILE << _T("h3 { color: #666666; text-align: center; font-family: Verdana, Helvetica, Arial, sans-serif; font-size: 16pt; }\n");
	EXP_FILE << _T("h4 { text-align: center; font-family: Verdana, Helvetica, Arial, sans-serif; font-size: 14pt; }\n");
	EXP_FILE << _T("h6 { font-weight: normal; color: #000000; text-align: center; font-family: Verdana, Helvetica, Arial, sans-serif; font-size: 8pt; }\n");
	EXP_FILE << _T(".mes { border-top-width: 1px; border-right-width: 0px; border-bottom-width: 0px;border-left-width: 0px; border-top-style: solid; border-right-style: solid; border-bottom-style: solid; border-left-style: solid; border-top-color: #666666; border-bottom-color: #666666; padding: 4px; color: #000000; font: normal normal normal 8pt normal Tahoma, Tahoma, Verdana, Arial, sans-serif; text-decoration: none; }\n");
	EXP_FILE << _T(".text { clear: both; }\n");
	EXP_FILE << _T(".nick { float: left; font: normal normal bold 8pt normal Tahoma, Tahoma, Verdana, Arial, sans-serif; text-decoration: none; }\n");
	EXP_FILE << _T(".date { float: right; clear: right; font: normal normal bold 8pt normal Tahoma, Tahoma, Verdana, Arial, sans-serif; text-decoration: none; }\n");
	EXP_FILE << _T(".url { color: #0000FF; }\n");
	EXP_FILE << _T(".nick#inc { color: #C83F6B; }\n");
	EXP_FILE << _T(".nick#out { color: #0860BD; }\n");
	EXP_FILE << _T(".date#inc { color: #000000; }\n");
	EXP_FILE << _T(".date#out { color: #000000; }\n");
	EXP_FILE << _T(".mes#event0 { background-color: #DBDBDB; }\n");
	EXP_FILE << _T(".mes#event1 { background-color: #EEEEEE; }\n");
	EXP_FILE << _T(".mes#event2 { background-color: #CCD9F4; }\n");
	EXP_FILE << _T(".mes#session { background-color: #FFFDD7; }\n");
	EXP_FILE << _T("--></style>\n</head><body>\n<h4>") << TranslateT("History Log") << _T("</h4>\n<h3>");
	EXP_FILE << MakeTextHtmled(myName);
	if (proto1.length() || myId.length())
		EXP_FILE << _T(" (") << MakeTextHtmled(proto1) << _T(": ") << MakeTextHtmled(myId) << _T(") - ");
	else
		EXP_FILE << _T(" - ");

	EXP_FILE << MakeTextHtmled(name1);
	if (proto1.length() || id1.length())
		EXP_FILE << _T(" (") << MakeTextHtmled(proto1) << _T(": ") << MakeTextHtmled(id1) << _T(")</h3>\n");
	else
		EXP_FILE << _T("</h3>\n");

	EXP_FILE << _T("<h6>") << TranslateT("Filter:") << _T(" ") << MakeTextHtmled(filterName) << _T("</h6>\n");
}

void PlainHtmlExport::WriteFooter()
{
	EXP_FILE << _T("<div class=mes></div>\n</body></html>\n");
}

void PlainHtmlExport::WriteGroup(bool isMe, const std::wstring &time, const std::wstring &user, const std::wstring &eventText)
{
	TCHAR buf[256];
	EXP_FILE << _T("<div class=mes id=session>\n");
	mir_sntprintf(buf, SIZEOF(buf), TranslateT("Conversation started at %s"), time.c_str());
	EXP_FILE << _T("<div class=text>") << buf << _T("</div>\n");
	EXP_FILE << _T("</div>\n");
}

void PlainHtmlExport::WriteMessage(bool isMe, const std::wstring &longDate, const std::wstring &shortDate, const std::wstring &user, const std::wstring &message, const DBEVENTINFO& dbei)
{
	TCHAR *id = isMe ? _T("out") : _T("inc");
	TCHAR* ev = (isMe ? _T("1") : _T("0"));
	bool isUrl = false;
	std::wstring& mes = UrlHighlightHtml(MakeTextHtmled(message), isUrl);
	if (isUrl)
		ev = _T("2");
	EXP_FILE << _T("<div class=mes id=event") << ev << _T(">\n");
	EXP_FILE << _T("<div class=nick id=") << id << _T(">") << MakeTextHtmled(user) << _T(":</div>\n");
	EXP_FILE << _T("<div class=date id=") << id << _T(">") << (Options::instance->exportHtml1ShowDate ? longDate : shortDate) << _T("</div>\n");
	EXP_FILE << _T("<div class=text>\n");
	EXP_FILE << mes;
	EXP_FILE << _T("\n</div>\n");
	EXP_FILE << _T("</div>\n");
}
