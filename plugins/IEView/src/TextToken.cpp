/*

IEView Plugin for Miranda IM
Copyright (C) 2005-2010  Piotr Piastucki

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "ieview_common.h"

TextToken::TextToken(int _type, const char *_text, int _len)
{
	next = NULL;
	tag = 0;
	end = false;
	type = _type;
	text = mir_strndup(_text, _len);
	wtext = mir_a2t(text);
	link = NULL;
	wlink = NULL;
}

TextToken::TextToken(int _type, const wchar_t *_wtext, int _len)
{
	next = NULL;
	tag = 0;
	end = false;
	type = _type;
	wtext = mir_tstrndup(_wtext, _len);
	text = mir_t2a(wtext);
	link = NULL;
	wlink = NULL;
}

TextToken::~TextToken()
{
	mir_free(text);
	mir_free(wtext);
	mir_free(link);
	mir_free(wlink);
}

void TextToken::setLink(const char *_link)
{
	replaceStr(link, _link);

	mir_free(wlink);
	this->wlink = mir_a2t(_link);
}

void TextToken::setLink(const wchar_t *_link)
{
	replaceStrW(wlink, _link);

	mir_free(link);
	link = mir_u2a(_link);
}

static int countNoWhitespace(const wchar_t *str)
{
	int c;
	for (c = 0; *str != '\n' && *str != '\r' && *str != '\t' && *str != ' ' && *str != '\0'; str++, c++);
	return c;
}

TextToken* TextToken::tokenizeBBCodes(const wchar_t *text)
{
	return tokenizeBBCodes(text, (int)mir_wstrlen(text));
}

// TODO: Add the following BBCodes: code
#define BB_TAG_NUM 10
TextToken* TextToken::tokenizeBBCodes(const wchar_t *text, int l)
{
	static const wchar_t *bbTagName[] = { L"b", L"i", L"u", L"s", L"img", L"color", L"size", L"bimg", L"url", L"code" };
	static int 		bbTagNameLen[] = { 1, 1, 1, 1, 3, 5, 4, 4, 3, 4 };
	static int 		bbTagArg[] = { 0, 0, 0, 0, 0, 1, 1, 0, 1, 0 };
	static int 		bbTagId[] = { BB_B, BB_I, BB_U, BB_S, BB_IMG, BB_COLOR, BB_SIZE, BB_BIMG, BB_URL, BB_CODE };
	static int      bbTagEnd[BB_TAG_NUM];
	static int      bbTagCount[BB_TAG_NUM];
	int i, j;
	TextToken *firstToken = NULL, *lastToken = NULL, *bbToken = NULL;
	int textLen = 0;
	for (j = 0; j < BB_TAG_NUM; j++) {
		bbTagCount[j] = 0;
		bbTagEnd[j] = 0;
	}
	for (i = 0; i <= l;) {
		int k, tagArgStart = 0, tagArgEnd = 0, tagDataStart = 0, newTokenType = 0, newTokenSize = 0;
		bool bbFound = false;
		if (text[i] == '[') {
			if (text[i + 1] != '/') {
				for (j = 0; j < BB_TAG_NUM; j++) {
					k = i + 1;
					if (!wcsnicmp(text + k, bbTagName[j], bbTagNameLen[j])) {
						tagArgStart = tagArgEnd = 0;
						k += bbTagNameLen[j];
						if (bbTagArg[j]) {
							if (text[k] != '=') continue;
							k++;
							tagArgStart = k;
							for (; text[k] != '\0'; k++) {
								if (text[k] == ']') break;
							}
							tagArgEnd = k;
						}
						if (text[k] == ']') {
							k++;
							tagDataStart = k;
							if (k < bbTagEnd[j]) k = bbTagEnd[j];
							for (; k < l; k++) {
								if (text[k] == '[' && text[k + 1] == '/') {
									k += 2;
									if (!wcsnicmp(text + k, bbTagName[j], bbTagNameLen[j])) {
										k += bbTagNameLen[j];
										if (text[k] == ']') {
											k++;
											bbFound = true;
											break;
										}
									}
								}
							}
							if (bbFound) break;
						}
					}
				}
				if (bbFound) {
					bbTagEnd[j] = k;
					switch (bbTagId[j]) {
					case BB_B:
					case BB_I:
					case BB_U:
					case BB_S:
					case BB_CODE:
					case BB_COLOR:
					case BB_SIZE:
						bbTagCount[j]++;
						if (bbTagArg[j]) {
							bbToken = new TextToken(BBCODE, text + tagArgStart, tagArgEnd - tagArgStart);
						}
						else {
							bbToken = new TextToken(BBCODE, bbTagName[j], bbTagNameLen[j]);
						}
						bbToken->setTag(bbTagId[j]);
						bbToken->setEnd(false);
						newTokenType = BBCODE;
						newTokenSize = tagDataStart - i;
						break;
					case BB_URL:
					case BB_IMG:
					case BB_BIMG:
						bbToken = new TextToken(BBCODE, text + tagDataStart, k - bbTagNameLen[j] - 3 - tagDataStart);
						bbToken->setTag(bbTagId[j]);
						bbToken->setEnd(false);
						newTokenType = BBCODE;
						newTokenSize = k - i;
						if (bbTagArg[j]) {
							wchar_t *urlLink = mir_tstrndup(text + tagArgStart, tagArgEnd - tagArgStart);
							bbToken->setLink(urlLink);
							mir_free(urlLink);
						}
						break;
					}
				}
			}
			else {
				for (j = 0; j < BB_TAG_NUM; j++) {
					k = i + 2;
					if (bbTagCount[j]>0 && !wcsnicmp(text + k, bbTagName[j], bbTagNameLen[j])) {
						k += bbTagNameLen[j];
						if (text[k] == ']') {
							k++;
							bbFound = true;
							break;
						}
					}
				}
				if (bbFound) {
					bbTagCount[j]--;
					bbToken = new TextToken(BBCODE, bbTagName[j], bbTagNameLen[j]);
					bbToken->setEnd(true);
					bbToken->setTag(bbTagId[j]);
					newTokenType = BBCODE;
					newTokenSize = k - i;
				}
			}
		}
		if (!bbFound) {
			if (i == l) {
				newTokenType = END;
				newTokenSize = 1;
			}
			else {
				newTokenType = TEXT;
				newTokenSize = 1;
			}
		}
		if (newTokenType != TEXT) {
			if (textLen > 0) {
				TextToken *newToken = new TextToken(TEXT, text + i - textLen, textLen);
				textLen = 0;
				if (lastToken == NULL)
					firstToken = newToken;
				else
					lastToken->setNext(newToken);
				lastToken = newToken;
			}
			if (newTokenType == BBCODE) {
				if (lastToken == NULL)
					firstToken = bbToken;
				else
					lastToken->setNext(bbToken);

				lastToken = bbToken;
			}
		}
		else textLen += newTokenSize;

		i += newTokenSize;
	}
	return firstToken;
}

TextToken* TextToken::tokenizeLinks(const wchar_t *text)
{
	TextToken *firstToken = NULL, *lastToken = NULL;
	int textLen = 0;
	int l = (int)mir_wstrlen(text);
	for (int i = 0; i <= l;) {
		int newTokenType, newTokenSize;
		int urlLen = Utils::detectURL(text + i);
		if (i == l) {
			newTokenType = END;
			newTokenSize = 1;
		}
		else if (urlLen > 0) {
			newTokenType = LINK;
			newTokenSize = urlLen;
		}
		else if (!wcsncmp(text + i, L"www.", 4)) {
			newTokenType = WWWLINK;
			newTokenSize = countNoWhitespace(text + i);
		}
		else if (!wcsncmp(text + i, L"mailto:", 7)) {
			newTokenType = LINK;
			newTokenSize = countNoWhitespace(text + i);
		}
		else {
			newTokenType = TEXT;
			newTokenSize = 1;
		}
		if (newTokenType != TEXT) {
			if (textLen > 0) {
				TextToken *newToken = new TextToken(TEXT, text + i - textLen, textLen);
				textLen = 0;
				if (lastToken == NULL)
					firstToken = newToken;
				else
					lastToken->setNext(newToken);

				lastToken = newToken;
			}
			if (newTokenType == WWWLINK || newTokenType == LINK) {
				TextToken *newToken = new TextToken(newTokenType, text + i, newTokenSize);
				newToken->setLink(newToken->getText());
				if (lastToken == NULL)
					firstToken = newToken;
				else
					lastToken->setNext(newToken);

				lastToken = newToken;
			}
		}
		else textLen += newTokenSize;

		i += newTokenSize;
	}
	return firstToken;
}

TextToken* TextToken::tokenizeSmileys(MCONTACT hContact, const char *proto, const wchar_t *text, bool isSent)
{
	TextToken *firstToken = NULL, *lastToken = NULL;
	int l = (int)mir_wstrlen(text);
	if (!Options::isSmileyAdd())
		return new TextToken(TEXT, text, l);

	SMADD_BATCHPARSE2 sp;
	sp.cbSize = sizeof(sp);
	sp.Protocolname = proto;
	sp.flag = SAFL_PATH | SAFL_UNICODE | (isSent ? SAFL_OUTGOING : 0);
	sp.wstr = (wchar_t *)text;
	sp.hContact = hContact;
	SMADD_BATCHPARSERES *spRes = (SMADD_BATCHPARSERES *)CallService(MS_SMILEYADD_BATCHPARSE, 0, (LPARAM)&sp);
	int last_pos = 0;
	if (spRes != NULL) {
		for (int i = 0; i < (int)sp.numSmileys; i++) {
			if (spRes[i].filepath != NULL && mir_strlen((char *)spRes[i].filepath) > 0) {
				if ((int)spRes[i].startChar - last_pos > 0) {
					TextToken *newToken = new TextToken(TEXT, text + last_pos, spRes[i].startChar - last_pos);
					if (lastToken == NULL)
						firstToken = newToken;
					else
						lastToken->setNext(newToken);

					lastToken = newToken;
				}
				TextToken *newToken = new TextToken(SMILEY, text + spRes[i].startChar, spRes[i].size);
				if (sp.oflag & SAFL_UNICODE)
					newToken->setLink((wchar_t *)spRes[i].filepath);
				else
					newToken->setLink((char *)spRes[i].filepath);

				if (lastToken == NULL)
					firstToken = newToken;
				else
					lastToken->setNext(newToken);

				lastToken = newToken;
				last_pos = spRes[i].startChar + spRes[i].size;
			}
		}
		CallService(MS_SMILEYADD_BATCHFREE, 0, (LPARAM)spRes);
	}
	if (last_pos < l) {
		TextToken *newToken = new TextToken(TEXT, text + last_pos, l - last_pos);
		if (lastToken == NULL)
			firstToken = newToken;
		else
			lastToken->setNext(newToken);

		lastToken = newToken;
	}
	return firstToken;
}

TextToken* TextToken::tokenizeChatFormatting(const wchar_t *text)
{
	TextToken *firstToken = NULL, *lastToken = NULL;
	int textLen = 0;
	int l = (int)mir_wstrlen(text);
	wchar_t* tokenBuffer = new wchar_t[l + 1];
	for (int i = 0; i <= l;) {
		int newTokenType = TEXT;
		int newTokenSize = 1;
		int newTokenTag = 0;
		int newTokenTextLen = 0;
		const wchar_t * newTokenText = NULL;
		bool endToken = false;


		if (i == l)
			newTokenType = END;
		else {
			if (text[i] == '%') {
				newTokenSize = 2;
				switch (text[i + 1]) {
				case '%':
					break;
				case 'B':
					endToken = true;
				case 'b':
					newTokenType = BBCODE;
					newTokenTag = BB_B;
					break;
				case 'U':
					endToken = true;
				case 'u':
					newTokenType = BBCODE;
					newTokenTag = BB_U;
					break;
				case 'I':
					endToken = true;
				case 'i':
					newTokenType = BBCODE;
					newTokenTag = BB_I;
					break;
				case 'C':
					endToken = true;
				case 'c':
					newTokenType = BBCODE;
					newTokenTag = BB_COLOR;
					if (!endToken) {
						newTokenText = text + i + 2;
						newTokenTextLen = 7;
						newTokenSize = 9;
					}
					break;
				case 'F':
					endToken = true;
				case 'f':
					newTokenType = BBCODE;
					newTokenTag = BB_BACKGROUND;
					if (!endToken) {
						newTokenText = text + i + 2;
						newTokenTextLen = 7;
						newTokenSize = 9;
					}
					break;
				default:
					newTokenSize = 1;
				}
			}
		}
		if (newTokenType != TEXT) {
			if (textLen > 0) {
				TextToken *newToken = new TextToken(TEXT, tokenBuffer, textLen);
				textLen = 0;
				if (lastToken == NULL)
					firstToken = newToken;
				else
					lastToken->setNext(newToken);

				lastToken = newToken;
			}
			if (newTokenType == BBCODE) {
				TextToken *newToken = new TextToken(newTokenType, newTokenText, newTokenTextLen);
				newToken->setEnd(endToken);
				newToken->setTag(newTokenTag);
				if (lastToken == NULL)
					firstToken = newToken;
				else
					lastToken->setNext(newToken);

				lastToken = newToken;
			}
		}
		else {
			tokenBuffer[textLen] = text[i];
			textLen++;
		}
		i += newTokenSize;
	}
	delete[] tokenBuffer;
	return firstToken;
}

wchar_t *TextToken::htmlEncode(const wchar_t *str)
{
	wchar_t *out;
	const wchar_t *ptr;
	if (str == NULL)
		return NULL;
	int c = 0;
	bool wasSpace = false;
	for (ptr = str; *ptr != '\0'; ptr++) {
		if (*ptr == ' ' && wasSpace)
			c += 6;
		else {
			wasSpace = false;
			switch (*ptr) {
			case '\n': c += 4; break;
			case '\r': break;
			case '&': c += 5; break;
			case '>': c += 4; break;
			case '<': c += 4; break;
			case '"': c += 6; break;
			case ' ': wasSpace = true;
			default: c++;
			}
		}
	}

	wasSpace = false;
	wchar_t *output = new wchar_t[c + 1];
	for (out = output, ptr = str; *ptr != '\0'; ptr++) {
		if (*ptr == ' ' && wasSpace) {
			mir_wstrcpy(out, L"&nbsp;");
			out += 6;
		}
		else {
			wasSpace = false;
			switch (*ptr) {
			case '\n': mir_wstrcpy(out, L"<br>"); out += 4; break;
			case '\r': break;
			case '&': mir_wstrcpy(out, L"&amp;"); out += 5; break;
			case '>': mir_wstrcpy(out, L"&gt;"); out += 4; break;
			case '<': mir_wstrcpy(out, L"&lt;"); out += 4; break;
			case '"': mir_wstrcpy(out, L"&quot;"); out += 6; break;
			case ' ': wasSpace = true;
			default: *out = *ptr; out++;
			}
		}
	}
	*out = '\0';
	return output;
}

void TextToken::toString(CMStringW &str)
{
	wchar_t *eText = NULL, *eLink = NULL;
	switch (type) {
	case TEXT:
		eText = htmlEncode(wtext);
		str.Append(eText);
		break;
	case WWWLINK:
	case LINK:
		eText = htmlEncode(wtext);
		eLink = htmlEncode(wlink);
		{
			const wchar_t *linkPrefix = type == WWWLINK ? L"http://" : L"";
			if ((Options::getGeneralFlags()&Options::GENERAL_ENABLE_EMBED)) {
				wchar_t *match = wcsstr(wlink, L"youtube.com");
				if (match != NULL) {
					match = wcsstr(match + 11, L"v=");
					if (match != NULL) {
						match += 2;
						wchar_t *match2 = wcsstr(match, L"&");
						int len = match2 != NULL ? match2 - match : (int)mir_wstrlen(match);
						match = mir_wstrdup(match);
						match[len] = 0;
						int width = 0;
						int height = 0;
						int Embedsize = Options::getEmbedsize();
						switch (Embedsize) {
						case 0:
							width = 320;
							height = 205;
							break;
						case 1:
							width = 480;
							height = 385;
							break;
						case 2:
							width = 560;
							height = 349;
							break;
						case 3:
							width = 640;
							height = 390;
							break;

						};

						str.AppendFormat(L"<div><object width=\"%d\" height=\"%d\">\
							<param name=\"movie\" value=\"http://www.youtube.com/v/%s&feature=player_embedded&version=3\"/>\
							<param name=\"allowFullScreen\" value=\"true\"/>\
							<param name=\"allowScriptAccess\" value=\"true\"/>\
							<embed src=\"http://www.youtube.com/v/%s&feature=player_embedded&version=3\" type=\"application/x-shockwave-flash\" allowfullscreen=\"true\" allowScriptAccess=\"always\" width=\"%d\" height=\"%d\"/>\
							</object></div>", width, height, match, match, width, height);
						mir_free(match);
						break;
					}
				}
			}
			str.AppendFormat(L"<a class=\"link\" target=\"_self\" href=\"%s%s\">%s</a>", linkPrefix, eLink, eText);
		}
		break;
	case SMILEY:
		eText = htmlEncode(wtext);
		if ((Options::getGeneralFlags()&Options::GENERAL_ENABLE_FLASH) && (wcsstr(wlink, L".swf") != NULL)) {
			str.AppendFormat(L"<span title=\"%s\" class=\"img\"><object classid=\"clsid:D27CDB6E-AE6D-11cf-96B8-444553540000\" \
				 	codebase=\"http://active.macromedia.com/flash2/cabs/swflash.cab#version=4,0,0,0\" >\
					<param NAME=\"movie\" VALUE=\"%s\"><param NAME=\"quality\" VALUE=\"high\"><PARAM NAME=\"loop\" VALUE=\"true\"></object></span>",
					eText, wlink);
		}
		else str.AppendFormat(L"<img class=\"img\" src=\"file://%s\" title=\"%s\" alt=\"%s\" />", wlink, eText, eText);
		break;
	case MATH:
		eText = htmlEncode(wtext);
		str.AppendFormat(L"<img class=\"img\" src=\"file://%s\" title=\"%s\" alt=\"%s\" />", wlink, eText, eText);
		break;
	case BBCODE:
		if (!end) {
			switch (tag) {
			case BB_B:
				str.Append(L"<b>");
				break;
			case BB_I:
				str.Append(L"<i>");
				break;
			case BB_U:
				str.Append(L"<u>");
				break;
			case BB_S:
				str.Append(L"<s>");
				break;
			case BB_CODE:
				str.Append(L"<pre class=\"code\">");
				break;
			case BB_IMG:
				eText = htmlEncode(wtext);
				if ((Options::getGeneralFlags()&Options::GENERAL_ENABLE_FLASH) && eText != NULL && (wcsstr(eText, L".swf") != NULL)) {
					str.AppendFormat(L"<div style=\"width: 100%%; border: 0; overflow: hidden;\"><object classid=\"clsid:D27CDB6E-AE6D-11cf-96B8-444553540000\" \
							codebase=\"http://active.macromedia.com/flash2/cabs/swflash.cab#version=4,0,0,0\" width=\"100%%\" >\
							<param NAME=\"movie\" VALUE=\"%s\"><param NAME=\"quality\" VALUE=\"high\"><PARAM NAME=\"loop\" VALUE=\"true\"></object></div>",
							eText);
				}
				else {
					if (eText != NULL && wcsncmp(eText, L"http://", 7) && wcsncmp(eText, L"https://", 8))
						str.AppendFormat(L"<div style=\"width: 100%%; border: 0; overflow: hidden;\"><img class=\"img\" style=\"width: expression((maxw = this.parentNode.offsetWidth ) > this.width ? 'auto' : maxw);\" src=\"file://%s\" /></div>", eText);
					else
						str.AppendFormat(L"<div style=\"width: 100%%; border: 0; overflow: hidden;\"><img class=\"img\" style=\"width: expression((maxw = this.parentNode.offsetWidth ) > this.width ? 'auto' : maxw);\" src=\"%s\" /></div>", eText);
				}
				break;
			case BB_BIMG:
				eText = htmlEncode(mir_ptr<wchar_t>(Utils::toAbsolute(wtext)));

				if ((Options::getGeneralFlags()&Options::GENERAL_ENABLE_FLASH) && (wcsstr(eText, L".swf") != NULL)) {
					str.AppendFormat(L"<div style=\"width: 100%%; border: 0; overflow: hidden;\"><object classid=\"clsid:D27CDB6E-AE6D-11cf-96B8-444553540000\" \
						 	codebase=\"http://active.macromedia.com/flash2/cabs/swflash.cab#version=4,0,0,0\" width=\"100%%\" >\
							<param NAME=\"movie\" VALUE=\"%s\"><param NAME=\"quality\" VALUE=\"high\"><PARAM NAME=\"loop\" VALUE=\"true\"></object></div>",
							eText);
				}
				else str.AppendFormat(L"<div style=\"width: 100%%; border: 0; overflow: hidden;\"><img class=\"img\" style=\"width: expression((maxw = this.parentNode.offsetWidth ) > this.width ? 'auto' : maxw);\" src=\"file://%s\" /></div>", eText);
				break;
			case BB_URL:
				eText = htmlEncode(wtext);
				eLink = htmlEncode(wlink);
				str.AppendFormat(L"<a href =\"%s\">%s</a>", eLink, eText);
				break;
			case BB_COLOR:
				eText = htmlEncode(wtext);
				str.AppendFormat(L"<font color =\"%s\">", eText);
				break;
			case BB_BACKGROUND:
				eText = htmlEncode(wtext);
				str.AppendFormat(L"<span style=\"background: %s;\">", eText);
				break;
			case BB_SIZE:
				eText = htmlEncode(wtext);
				str.AppendFormat(L"<span style=\"font-size: %s;\">", eText);
				break;
			}
		}
		else {
			switch (tag) {
			case BB_B:
				str.Append(L"</b>");
				break;
			case BB_I:
				str.Append(L"</i>");
				break;
			case BB_U:
				str.Append(L"</u>");
				break;
			case BB_S:
				str.Append(L"</s>");
				break;
			case BB_CODE:
				str.Append(L"</pre>");
				break;
			case BB_COLOR:
				str.Append(L"</font>");
				break;
			case BB_SIZE:
				str.Append(L"</span>");
				break;
			case BB_BACKGROUND:
				str.Append(L"</span>");
				break;
			}
		}
		break;
	}
	if (eText != NULL) delete[] eText;
	if (eLink != NULL) delete[] eLink;
}
