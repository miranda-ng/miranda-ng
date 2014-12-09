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

PasteFormat PasteToWeb1::formats[] =
{
	{L"4cs", L"4CS"},
	{L"6502acme", L"6502 ACME Cross Assembler"},
	{L"6502kickass", L"6502 Kick Assembler"},
	{L"6502tasm", L"6502 TASM/64TASS"},
	{L"abap", L"ABAP"},
	{L"actionscript", L"ActionScript"},
	{L"actionscript3", L"ActionScript 3"},
	{L"ada", L"Ada"},
	{L"algol68", L"ALGOL 68"},
	{L"apache", L"Apache Log"},
	{L"applescript", L"AppleScript"},
	{L"apt_sources", L"APT Sources"},
	{L"asm", L"ASM (NASM)"},
	{L"asp", L"ASP"},
	{L"autoconf", L"autoconf"},
	{L"autohotkey", L"Autohotkey"},
	{L"autoit", L"AutoIt"},
	{L"avisynth", L"Avisynth"},
	{L"awk", L"Awk"},
	{L"bascomavr", L"BASCOM AVR"},
	{L"bash", L"Bash"},
	{L"basic4gl", L"Basic4GL"},
	{L"bibtex", L"BibTeX"},
	{L"blitzbasic", L"Blitz Basic"},
	{L"bnf", L"BNF"},
	{L"boo", L"BOO"},
	{L"bf", L"BrainFuck"},
	{L"c", L"C"},
	{L"c_mac", L"C for Macs"},
	{L"cil", L"C Intermediate Language"},
	{L"csharp", L"C#"},
	{L"cpp", L"C++"},
	{L"cpp-qt", L"C++ (with QT extensions)"},
	{L"c_loadrunner", L"C: Loadrunner"},
	{L"caddcl", L"CAD DCL"},
	{L"cadlisp", L"CAD Lisp"},
	{L"cfdg", L"CFDG"},
	{L"chaiscript", L"ChaiScript"},
	{L"clojure", L"Clojure"},
	{L"klonec", L"Clone C"},
	{L"klonecpp", L"Clone C++"},
	{L"cmake", L"CMake"},
	{L"cobol", L"COBOL"},
	{L"coffeescript", L"CoffeeScript"},
	{L"cfm", L"ColdFusion"},
	{L"css", L"CSS"},
	{L"cuesheet", L"Cuesheet"},
	{L"d", L"D"},
	{L"dcs", L"DCS"},
	{L"delphi", L"Delphi"},
	{L"oxygene", L"Delphi Prism (Oxygene)"},
	{L"diff", L"Diff"},
	{L"div", L"DIV"},
	{L"dos", L"DOS"},
	{L"dot", L"DOT"},
	{L"e", L"E"},
	{L"ecmascript", L"ECMAScript"},
	{L"eiffel", L"Eiffel"},
	{L"email", L"Email"},
	{L"epc", L"EPC"},
	{L"erlang", L"Erlang"},
	{L"fsharp", L"F#"},
	{L"falcon", L"Falcon"},
	{L"fo", L"FO Language"},
	{L"f1", L"Formula One"},
	{L"fortran", L"Fortran"},
	{L"freebasic", L"FreeBasic"},
	{L"gambas", L"GAMBAS"},
	{L"gml", L"Game Maker"},
	{L"gdb", L"GDB"},
	{L"genero", L"Genero"},
	{L"genie", L"Genie"},
	{L"gettext", L"GetText"},
	{L"go", L"Go"},
	{L"groovy", L"Groovy"},
	{L"gwbasic", L"GwBasic"},
	{L"haskell", L"Haskell"},
	{L"hicest", L"HicEst"},
	{L"hq9plus", L"HQ9 Plus"},
	{L"html4strict", L"HTML"},
	{L"html5", L"HTML 5"},
	{L"icon", L"Icon"},
	{L"idl", L"IDL"},
	{L"ini", L"INI file"},
	{L"inno", L"Inno Script"},
	{L"intercal", L"INTERCAL"},
	{L"io", L"IO"},
	{L"j", L"J"},
	{L"java", L"Java"},
	{L"java5", L"Java 5"},
	{L"javascript", L"JavaScript"},
	{L"jquery", L"jQuery"},
	{L"kixtart", L"KiXtart"},
	{L"latex", L"Latex"},
	{L"lb", L"Liberty BASIC"},
	{L"lsl2", L"Linden Scripting"},
	{L"lisp", L"Lisp"},
	{L"llvm", L"LLVM"},
	{L"locobasic", L"Loco Basic"},
	{L"logtalk", L"Logtalk"},
	{L"lolcode", L"LOL Code"},
	{L"lotusformulas", L"Lotus Formulas"},
	{L"lotusscript", L"Lotus Script"},
	{L"lscript", L"LScript"},
	{L"lua", L"Lua"},
	{L"m68k", L"M68000 Assembler"},
	{L"magiksf", L"MagikSF"},
	{L"make", L"Make"},
	{L"mapbasic", L"MapBasic"},
	{L"matlab", L"MatLab"},
	{L"mirc", L"mIRC"},
	{L"mmix", L"MIX Assembler"},
	{L"modula2", L"Modula 2"},
	{L"modula3", L"Modula 3"},
	{L"68000devpac", L"Motorola 68000 HiSoft Dev"},
	{L"mpasm", L"MPASM"},
	{L"mxml", L"MXML"},
	{L"mysql", L"MySQL"},
	{L"newlisp", L"newLISP"},
	{L"text", L"None"},
	{L"nsis", L"NullSoft Installer"},
	{L"oberon2", L"Oberon 2"},
	{L"objeck", L"Objeck Programming Langua"},
	{L"objc", L"Objective C"},
	{L"ocaml-brief", L"OCalm Brief"},
	{L"ocaml", L"OCaml"},
	{L"pf", L"OpenBSD PACKET FILTER"},
	{L"glsl", L"OpenGL Shading"},
	{L"oobas", L"Openoffice BASIC"},
	{L"oracle11", L"Oracle 11"},
	{L"oracle8", L"Oracle 8"},
	{L"oz", L"Oz"},
	{L"pascal", L"Pascal"},
	{L"pawn", L"PAWN"},
	{L"pcre", L"PCRE"},
	{L"per", L"Per"},
	{L"perl", L"Perl"},
	{L"perl6", L"Perl 6"},
	{L"php", L"PHP"},
	{L"php-brief", L"PHP Brief"},
	{L"pic16", L"Pic 16"},
	{L"pike", L"Pike"},
	{L"pixelbender", L"Pixel Bender"},
	{L"plsql", L"PL/SQL"},
	{L"postgresql", L"PostgreSQL"},
	{L"povray", L"POV-Ray"},
	{L"powershell", L"Power Shell"},
	{L"powerbuilder", L"PowerBuilder"},
	{L"proftpd", L"ProFTPd"},
	{L"progress", L"Progress"},
	{L"prolog", L"Prolog"},
	{L"properties", L"Properties"},
	{L"providex", L"ProvideX"},
	{L"purebasic", L"PureBasic"},
	{L"pycon", L"PyCon"},
	{L"python", L"Python"},
	{L"q", L"q/kdb+"},
	{L"qbasic", L"QBasic"},
	{L"rsplus", L"R"},
	{L"rails", L"Rails"},
	{L"rebol", L"REBOL"},
	{L"reg", L"REG"},
	{L"robots", L"Robots"},
	{L"rpmspec", L"RPM Spec"},
	{L"ruby", L"Ruby"},
	{L"gnuplot", L"Ruby Gnuplot"},
	{L"sas", L"SAS"},
	{L"scala", L"Scala"},
	{L"scheme", L"Scheme"},
	{L"scilab", L"Scilab"},
	{L"sdlbasic", L"SdlBasic"},
	{L"smalltalk", L"Smalltalk"},
	{L"smarty", L"Smarty"},
	{L"sql", L"SQL"},
	{L"systemverilog", L"SystemVerilog"},
	{L"tsql", L"T-SQL"},
	{L"tcl", L"TCL"},
	{L"teraterm", L"Tera Term"},
	{L"thinbasic", L"thinBasic"},
	{L"typoscript", L"TypoScript"},
	{L"unicon", L"Unicon"},
	{L"uscript", L"UnrealScript"},
	{L"vala", L"Vala"},
	{L"vbnet", L"VB.NET"},
	{L"verilog", L"VeriLog"},
	{L"vhdl", L"VHDL"},
	{L"vim", L"VIM"},
	{L"visualprolog", L"Visual Pro Log"},
	{L"vb", L"VisualBasic"},
	{L"visualfoxpro", L"VisualFoxPro"},
	{L"whitespace", L"WhiteSpace"},
	{L"whois", L"WHOIS"},
	{L"winbatch", L"Win Batch"},
	{L"xbasic", L"XBasic"},
	{L"xml", L"XML"},
	{L"xorg_conf", L"Xorg Config"},
	{L"xpp", L"XPP"},
	{L"yaml", L"YAML"},
	{L"z80", L"Z80 Assembler"},
	{L"zxbasic", L"ZXBasic"},
};

PasteFormat PasteToWeb1::defFormats[] =
{
	{L"text", L"None"},
	{L"xml", L"XML"},
	{L"html4strict", L"HTML"},
	{L"html5", L"HTML 5"},
	{L"javascript", L"JavaScript"},
	{L"php", L"PHP"},
	{L"c", L"C"},
	{L"csharp", L"C#"},
	{L"cpp", L"C++"},
	{L"java", L"Java"},
	{L"java5", L"Java 5"},
	{L"ini", L"INI file"},
	{L"css", L"CSS"},
	{L"sql", L"SQL"},
	{L"asm", L"ASM (NASM)"},
	{L"asp", L"ASP"},
	{L"caddcl", L"CAD DCL"},
	{L"cadlisp", L"CAD Lisp"},
	{L"delphi", L"Delphi"},
	{L"diff", L"Diff"},
	{L"jquery", L"jQuery"},
	{L"latex", L"Latex"},
	{L"pascal", L"Pascal"},
	{L"perl", L"Perl"},
	{L"perl6", L"Perl 6"},
	{L"python", L"Python"},
	{L"vbnet", L"VB.NET"},
	{L"vb", L"VisualBasic"},
};

PasteToWeb1::PasteToWeb1()
{
}


PasteToWeb1::~PasteToWeb1()
{
}

void PasteToWeb1::SendToServer(std::wstring str, std::wstring fileName, std::wstring format)
{
	std::map<std::string, std::string> headers;
	headers["Content-Type"] = "application/x-www-form-urlencoded";
	std::wstring content = _T("api_option=paste&api_paste_private=");
	content += Options::instance->webOptions[pageIndex]->publicPaste ? _T("0") : _T("1");
	content += _T("&api_paste_expire_date=");
	content += Options::instance->webOptions[pageIndex]->combo1.empty() ? _T("1M") : Options::instance->webOptions[pageIndex]->combo1;
	content += _T("&api_dev_key=dcba056bf9cc71729fdad76dddcb0dcd&api_paste_format=");
	content += format;
	if(!Options::instance->webOptions[pageIndex]->pastebinUserKey.empty())
	{
		content += _T("&api_user_key=");
		content += Options::instance->webOptions[pageIndex]->pastebinUserKey;
	}
	content += _T("&api_paste_code=");
	for(std::wstring::iterator it = str.begin(); it != str.end(); ++it)
	{
		if(*it == L'%')
		{
			content += L"%25";
		}
		else if(*it ==L'&')
		{
			content += L"%26";
		}
		else if(*it ==L'=')
		{
			content += L"%3D";
		}
		else
		{
			content += *it;
		}
	}
	
	wchar_t* resCont = SendToWeb("http://pastebin.com/api/api_post.php", headers, content);
	if(resCont != NULL)
	{
		if(memcmp(L"Bad API request, ", resCont, 17 * sizeof(wchar_t)) == 0)
		{
			mir_sntprintf(bufErr, SIZEOF(bufErr), TranslateT("Error during sending text to web page: %s"), resCont + 17);
			error = bufErr;
		}
		else
		{
			char* s = mir_u2a_cp(resCont, CP_ACP);
			strcpy_s(szFileLink, 256, s);
			mir_free(s);
		}
		mir_free(resCont);
	}
	else
	{
		error = TranslateT("Error during sending text to web page");
	}
}

std::wstring PasteToWeb1::GetUserKey(std::wstring& user, std::wstring& password)
{
	std::map<std::string, std::string> headers;
	headers["Content-Type"] = "application/x-www-form-urlencoded";
	std::wstring content = _T("api_user_name=");
	content += user;
	content += _T("&api_user_password=");
	content += password;
	content += _T("&api_dev_key=dcba056bf9cc71729fdad76dddcb0dcd");
	wchar_t* resCont = SendToWeb("http://pastebin.com/api/api_login.php", headers, content);
	std::wstring toRet;
	if(resCont != NULL)
	{
		if(memcmp(L"Bad API request, ", resCont, 17 * sizeof(wchar_t)) == 0)
		{
			mir_sntprintf(bufErr, SIZEOF(bufErr), TranslateT("Error during getting user key from web page: %s"), resCont + 17);
			MessageBox(NULL, bufErr, TranslateT("Error"), MB_OK | MB_ICONERROR);
		}
		else
			toRet = resCont;
		mir_free(resCont);
	}

	return toRet;
}

std::list<PasteFormat> PasteToWeb1::GetFormats()
{
	int count = sizeof(formats) / sizeof(formats[0]);
	std::list<PasteFormat> ret(formats, formats + count);

	return ret;
}

std::list<PasteFormat> PasteToWeb1::GetDefFormats()
{
	int count = sizeof(defFormats) / sizeof(defFormats[0]);
	std::list<PasteFormat> ret(defFormats, defFormats + count);

	return ret;
}

void PasteToWeb1::ConfigureSettings()
{
	Options::instance->webOptions[pageIndex]->isSendFileName = false;
	Options::instance->webOptions[pageIndex]->isPublicPaste = true;
	Options::instance->webOptions[pageIndex]->isCombo1 = true;
	Options::instance->webOptions[pageIndex]->combo1Desc = TranslateT("Paste expire date");
	PasteFormat pf;
	pf.id = L"N";
	pf.name = TranslateT("Never");
	Options::instance->webOptions[pageIndex]->combo1Values.push_back(pf);
	pf.id = L"10M";
	pf.name = TranslateT("10 minutes");
	Options::instance->webOptions[pageIndex]->combo1Values.push_back(pf);
	pf.id = L"1H";
	pf.name = TranslateT("1 hour");
	Options::instance->webOptions[pageIndex]->combo1Values.push_back(pf);
	pf.id = L"1D";
	pf.name = TranslateT("1 day");
	Options::instance->webOptions[pageIndex]->combo1Values.push_back(pf);
	pf.id = L"1M";
	pf.name = TranslateT("1 month");
	Options::instance->webOptions[pageIndex]->combo1Values.push_back(pf);
	Options::instance->webOptions[pageIndex]->combo1 = L"1M";
	Options::instance->webOptions[pageIndex]->isPastebin = true;
}

