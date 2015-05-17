/*
Paste It plugin
Copyright (C) 2011 Krzysztof Kral

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

PasteFormat PasteToWeb2::defFormats[] =
{
	{ L"text", L"Text" },
	{ L"xml", L"XML" },
	{ L"html", L"HTML" },
	{ L"js", L"JavaScript" },
	{ L"php", L"PHP" },
	{ L"c", L"C" },
	{ L"csharp", L"C#" },
	{ L"cpp", L"C++" },
	{ L"java", L"Java" },
	{ L"ini", L"INI" },
	{ L"css", L"CSS" },
	{ L"sql", L"SQL" },
	{ L"nasm", L"NASM" },
	{ L"delphi", L"Delphi" },
	{ L"diff", L"Diff" },
	{ L"tex", L"TeX" },
	{ L"perl", L"Perl" },
	{ L"python", L"Python" },
	{ L"vb.net", L"VB.net" }
};

PasteToWeb2::PasteToWeb2()
{
}


PasteToWeb2::~PasteToWeb2()
{
}

void PasteToWeb2::SendToServer(std::wstring str, std::wstring fileName, std::wstring format)
{
	std::map<std::string, std::string> headers;
	headers["Content-Type"] = "text/xml";
	std::wstring content = _T("<?xml version=\"1.0\"?>\r\n<methodCall><methodName>create_paste</methodName><params><param><value>");
	if (fileName == L"")
	{
		content += format;
		content += _T("</value></param><param><value>");
	}
	else
	{
		content += _T("</value></param><param><value>");
	}
	for (std::wstring::iterator it = str.begin(); it != str.end(); ++it)
	{
		if (*it == L'&')
		{
			content += L"&amp;";
		}
		else if (*it == L'<')
		{
			content += L"&lt;";
		}
		else
		{
			content += *it;
		}
	}

	content += _T("</value></param><param><value></value></param><param><value>");
	if (fileName != L"")
	{
		for (std::wstring::iterator it = fileName.begin(); it != fileName.end(); ++it)
		{
			if (*it == L'&')
			{
				content += L"&amp;";
			}
			else if (*it == L'<')
			{
				content += L"&lt;";
			}
			else
			{
				content += *it;
			}
		}
	}
	content += _T("</value></param><param><value></value></param><param><value><double>1.5</double></value></param></params></methodCall>");

	wchar_t* resCont = SendToWeb("http://wklej.to/api/", headers, content);
	error = TranslateT("Error during sending text to web page");
	if (resCont != NULL)
	{
		HXML hXml = xi.parseString(resCont, NULL, _T("methodResponse"));
		if (hXml != NULL)
		{
			HXML node = xi.getChildByPath(hXml, _T("params/param/value/array/data/value/int"), 0);
			if (node != NULL && !mir_tstrcmp(xi.getText(node), _T("1")))
			{
				node = xi.getChildByPath(hXml, _T("params/param/value/array/data"), 0);
				if (node != NULL)
				{
					node = xi.getNthChild(node, _T("value"), 1);
					if (node != NULL)
					{
						node = xi.getChildByPath(node, _T("string"), 0);
						if (node != NULL)
						{
							char* s = mir_t2a_cp(xi.getText(node), CP_ACP);
							mir_strncpy(szFileLink, s, SIZEOF(szFileLink));
							mir_free(s);
							error = NULL;
						}
					}
				}
			}
			xi.destroyNode(hXml);
		}
		mir_free(resCont);
	}
}

std::list<PasteFormat> PasteToWeb2::GetFormats()
{
	std::list<PasteFormat> ret;

	std::map<std::string, std::string> headers;
	headers["Content-Type"] = "text/xml";
	std::wstring content = _T("<?xml version=\"1.0\"?>\r\n<methodCall><methodName>types</methodName></methodCall>");

	wchar_t* resCont = SendToWeb("http://wklej.to/api/", headers, content);
	if (resCont != NULL)
	{
		HXML hXml = xi.parseString(resCont, NULL, _T("methodResponse"));
		if (hXml != NULL)
		{
			HXML node = xi.getChildByPath(hXml, _T("params/param/value/array/data/value/int"), 0);
			if (node != NULL && !mir_tstrcmp(xi.getText(node), _T("1")))
			{
				node = xi.getChildByPath(hXml, _T("params/param/value/array/data"), 0);
				if (node != NULL)
				{
					node = xi.getNthChild(node, _T("value"), 1);
					if (node != NULL)
					{
						node = xi.getChildByPath(node, _T("string"), 0);
						if (node != NULL)
						{
							std::wstring str = xi.getText(node);
							std::wstring::size_type pos = str.find(L'\n');
							if (pos < str.length())
							{
								str = str.substr(pos + 1);
							}
							pos = str.find(L'\n');
							if (pos < str.length())
							{
								str = str.substr(pos + 1);
							}
							pos = str.find(L'\n');
							while (pos < str.length())
							{
								std::wstring line = str.substr(0, pos);
								std::wstring::size_type sep = line.find(L':');
								if (sep < line.length())
								{
									PasteFormat pf;
									pf.name = line.substr(0, sep);
									std::wstring::size_type sep2 = line.find(L',');
									if (sep2 < line.length())
									{
										pf.id = line.substr(sep + 2, sep2 - sep - 2);
									}
									else
									{
										pf.id = line.substr(sep + 2);
									}
									ret.push_back(pf);
								}

								if (pos < str.length() - 1)
								{
									str = str.substr(pos + 1);
								}
								else
								{
									str = L"";
								}
								pos = str.find(L'\n');
							}
							{
								std::wstring line = str;
								std::wstring::size_type sep = line.find(L':');
								if (sep < line.length())
								{
									PasteFormat pf;
									pf.name = line.substr(0, sep);
									std::wstring::size_type sep2 = line.find(L',');
									if (sep2 < line.length())
									{
										pf.id = line.substr(sep + 2, sep2 - sep - 2);
									}
									else
									{
										pf.id = line.substr(sep + 2);
									}
									ret.push_back(pf);
								}
							}
						}
					}
				}
			}
			xi.destroyNode(hXml);
		}
		mir_free(resCont);
	}

	return ret;
}

std::list<PasteFormat> PasteToWeb2::GetDefFormats()
{
	int count = sizeof(defFormats) / sizeof(defFormats[0]);
	std::list<PasteFormat> ret(defFormats, defFormats + count);

	return ret;
}

void PasteToWeb2::ConfigureSettings()
{
}
