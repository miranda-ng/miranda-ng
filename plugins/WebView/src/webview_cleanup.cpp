/*
* A plugin for Miranda IM which displays web page text in a window Copyright 
* (C) 2005 Vincent Joyce.
* 
* Miranda IM: the free icq client for MS Windows  Copyright (C) 2000-2
* Richard Hughes, Roland Rabien & Tristan Van de Vreede
* 
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the Free 
* Software Foundation; either version 2 of the License, or (at your option)
* any later version.
* 
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
* or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
* 
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc., 59 
* Temple Place - Suite 330, Boston, MA  02111-1307, USA. 
*/

#include "stdafx.h"
#include "webview.h"

// ///////////////////////
// characters and symbols//
// ///////////////////////

#define AMOUNT3  164

char*CharacterCodes[AMOUNT3] =
{
	"&quot;",
	"&amp;",
	"&lt;",
	"&gt;",
	"&nbsp;",
	"&iexcl;",
	"&cent;",
	"&pound;",
	"&curren;",
	"&yen;",
	"&brvbar",
	"&sect;",
	"&uml;",
	"&copy;",
	"&ordf;",
	"&laquo;",
	"&not;",
	"&shy;",
	"&reg;",
	"&macr;",
	"&deg;",
	"&plusmn;",
	"&sup2;",
	"&sup3;",
	"&acute;",
	"&micro;",
	"&para;",
	"&middot;",
	"&cedil;",
	"&sup1;",
	"&ordm;",
	"&raquo;",
	"&frac14;",
	"&frac12;",
	"&frac34;",
	"&iquest;",
	"&Agrave;",
	"&Aacute;",
	"&Acirc;",
	"&Atilde;",
	"&Auml;",
	"&Aring;",
	"&AElig;",
	"&Ccedil;",
	"&Egrave;",
	"&Eacute;",
	"&Ecirc;",
	"&Euml;",
	"&Igrave;",
	"&Iacute;",
	"&Icirc;",
	"&Iuml;",
	"&ETH;",
	"&Ntilde;",
	"&Ograve;",
	"&Oacute;",
	"&Ocirc;",
	"&Otilde;",
	"&Ouml;",
	"&times;",
	"&Oslash;",
	"&Ugrave;",
	"&Uacute;",
	"&Ucirc;",
	"&Uuml;",
	"&Yacute;",
	"&THORN;",
	"&szlig;",
	"&agrave;",
	"&aacute;",
	"&acirc;",
	"&atilde;",
	"&auml;",
	"&aring;",
	"&aelig;",
	"&ccedil;",
	"&egrave;",
	"&eacute;",
	"&ecirc;",
	"&euml;",
	"&igrave;",
	"&iacute;",
	"&icirc;",
	"&iuml;",
	"&eth;",
	"&ntilde;",
	"&ograve;",
	"&oacute;",
	"&ocirc;",
	"&otilde;",
	"&ouml;",
	"&divide;",
	"&oslash;",
	"&ugrave;",
	"&uacute;",
	"&ucirc;",
	"&uuml;",
	"&yacute;",
	"&thorn;",
	"&yumil;",
	"&#338;",        // greater that 255, extra latin characters
	"&#339;",
	"&#352;",
	"&#353;",
	"&#376;",
	"&#402;",
	"&#710;",
	"&#732;",
	"&OElig;",
	"&oelig;",
	"&Scaron;",
	"&scaron;",
	"&Yuml;",
	"&fnof;",
	"&circ;",
	"&tilde;",
	"&#8211;",       // Misc other characters
	"&#8212;",
	"&#8216;",
	"&#8217;",
	"&#8218;",
	"&#8220;",
	"&#8221;",
	"&#8222;",
	"&#8224;",
	"&#8225;",
	"&#8226;",
	"&#8230;",
	"&#8240;",
	"&#8249;",
	"&#8250;",
	"&#8364;",
	"&#8465;",
	"&#8476;",
	"&#8482;",
	"&ndash;",
	"&mdash;",
	"&lsquo;",
	"&rsquo;",
	"&sbquo;",
	"&ldquo;",
	"&rdquo;",
	"&bdquo;",
	"&dagger;",
	"&Dagger;",
	"&bull;",
	"&hellip;",
	"&permil;",
	"&lsaquo;",
	"&rsaquo;",
	"&euro;",
	"&image;",
	"&real;",
	"&trade;",
	"&ensp;",
	"&emsp;",
	"&thinsp;",
	"&#8194;",
	"&#8195;",
	"&#8201;",
	"&otilde;",      // symbols without numeric code
	"&iquest;",
	"&brvbar;",
	"&macr;"};

char Characters[AMOUNT3] =
{
	'\"',
	'&',
	'<',
	'>',
	' ',
	'¡',
	'¢',
	'£',
	'¤',
	'¥',
	'¦',
	'§',
	'¨',
	'©',
	'ª',
	'«',
	'¬',
	'­',
	'®',
	'¯',
	'°',
	'±',
	'²',
	'³',
	'´',
	'µ',
	'¶',
	'·',
	'¸',
	'¹',
	'º',
	'»',
	'¼',
	'½',
	'¾',
	'¿',
	'À',
	'Á',
	'Â',
	'Ã',
	'Ä',
	'Å',
	'Æ',
	'Ç',
	'È',
	'É',
	'Ê',
	'Ë',
	'Ì',
	'Í',
	'Î',
	'Ï',
	'Ð',
	'Ñ',
	'Ò',
	'Ó',
	'Ô',
	'Õ',
	'Ö',
	'×',
	'Ø',
	'Ù',
	'Ú',
	'Û',
	'Ü',
	'Ý',
	'Þ',
	'ß',
	'à',
	'á',
	'â',
	'ã',
	'ä',
	'å',
	'æ',
	'ç',
	'è',
	'é',
	'ê',
	'ë',
	'ì',
	'í',
	'î',
	'ï',
	'ð',
	'ñ',
	'ò',
	'ó',
	'ô',
	'õ',
	'ö',
	'÷',
	'ø',
	'ù',
	'ú',
	'û',
	'ü',
	'ý',
	'þ',
	'ÿ',
	'Œ',  // greater than 255 extra latin characters
	'œ',
	'Š',
	'š',
	'Ÿ',
	'ƒ',
	'ˆ',
	'˜',
	'Œ',
	'œ',
	'Š',
	'š',
	'Ÿ',
	'ƒ',
	'ˆ',
	'˜',
	'–',
	'—',  // misc other characters
	'‘',
	'’',
	'‚',
	'“',
	'”',
	'„',
	'†',
	'‡',
	'•',
	'…',
	'‰',
	'‹',
	'›',
	'€',
	'I',
	'R',
	'™',
	'–',
	'—',
	'‘',
	'’',
	'‚',
	'“',
	'”',
	'„',
	'†',
	'‡',
	'•',
	'…',
	'‰',
	'‹',
	'›',
	'€',
	'I',
	'R',
	'™',
	' ',
	' ',
	' ',
	' ',
	' ',
	' ',
	'õ',
	'¿',
	'¦',
	'¯'};

/*****************************************************************************/
void CodetoSymbol(char *truncated)
{
	int counter = 0;
	int position = 0, recpos = 0;
	static char *stringfrompos;

	for (int n = 0; n < AMOUNT3; n++) {
		while (true) { // loop forever
			Sleep(1); // avoid 100% CPU

			if ( strstr(truncated, CharacterCodes[n]) != 0) { // does character code exist?
				if (strstr(truncated, CharacterCodes[n]) != 0)
					stringfrompos = strstr(truncated, CharacterCodes[n]);

				position = stringfrompos - truncated;
				counter = 0;
				while (counter != mir_strlen(CharacterCodes[n])) {
					truncated[position + counter] = ' ';
					counter++;
				}

				truncated[(position + counter) - 1] = Characters[n];
				mir_strncpy(&truncated[position], &truncated[position + mir_strlen(CharacterCodes[n])] - 1, mir_strlen(&truncated[position]) - 1);
			} // end does character code exist?

			if (recpos == position)
				break; // break out of loop if doesn't find new character code

			recpos = position;
		} // end loop forever         
	} // for
}

/*****************************************************************************/
void EraseBlock(char *truncated)
{
	int counter = 0;
	int positionStart = 0, positionEnd = 0;
	char *stringfrompos;
	int BlockLength = 0;

	char* tempraw = (char*)malloc(MAXSIZE1);
	if (truncated)
		mir_strncpy(tempraw, truncated, MAXSIZE1);

	// ///////////////////////////

	while (true) {
		Sleep(1); // avoid 100% CPU
		// /get start and end of block

		if (strstr(tempraw, "<!--") != 0) // does tag exist?
		{
			stringfrompos = strstr(tempraw, "<!--");
			positionStart = stringfrompos - tempraw;
		}

		if (strstr(tempraw, "-->") != 0) // does tag exist?
		{
			stringfrompos = strstr(tempraw, "-->");
			positionEnd = stringfrompos - tempraw;
		}
		BlockLength = (positionEnd - positionStart) + 3;

		if ((strstr(tempraw, "<!--") == 0) || (strstr(tempraw, "-->") == 0))
			break;

		/////////////////////////////////////////

		if (strstr(tempraw, "<!--") != 0)
			for (counter = 0; counter < BlockLength; counter++)
				tempraw[positionStart + counter] = ' ';

		if ((positionStart == 0) && (positionEnd == 0))
			break;
		if (positionStart > positionEnd)
			break;

		positionStart = 0;
		positionEnd = 0;
	}

	// ///////////////////////////
	positionStart = 0;
	positionEnd = 0;

	// 2//
	while (true) {
		Sleep(1); // avoid 100% CPU
		// /get start and end of block

		if (((strstr(tempraw, "<script")) != 0) || (strstr(tempraw, "<SCRIPT") != 0)) // does 
		{
			if (strstr(tempraw, "<script") != 0)
				stringfrompos = strstr(tempraw, "<script");
			else
				stringfrompos = strstr(tempraw, "<SCRIPT");

			positionStart = stringfrompos - tempraw;
		}

		if (((strstr(tempraw, "</script")) != 0) || (strstr(tempraw, "</SCRIPT") != 0)) // does 
		{
			if (strstr(tempraw, "<script") != 0)
				stringfrompos = strstr(tempraw, "</script");
			else
				stringfrompos = strstr(tempraw, "</SCRIPT");

			positionEnd = stringfrompos - tempraw;
		}
		BlockLength = (positionEnd - positionStart) + 9;

		if ((strstr(tempraw, "<script") != 0) && (strstr(tempraw, "</script") == 0))
			break;

		if ((strstr(tempraw, "<SCRIPT") != 0) && (strstr(tempraw, "</SCRIPT") == 0))
			break;

		///////////////////////////////////////

		if (((strstr(tempraw, "<script")) != 0) || ((strstr(tempraw, "<SCRIPT")) != 0))
			for (counter = 0; counter < BlockLength; counter++)
				tempraw[positionStart + counter] = ' ';

		if ((positionStart == 0) && (positionEnd == 0))
			break;
		if (positionStart > positionEnd)
			break;

		positionStart = 0;
		positionEnd = 0;
	}

	// ////
	// 3//
	while (true)
	{
		Sleep(1); // avoid 100% CPU
		// /get start and end of block

		if (((strstr(tempraw, "<style")) != 0) || (strstr(tempraw, "<STYLE") != 0)) // does 
		{
			if (strstr(tempraw, "<style") != 0)
				stringfrompos = strstr(tempraw, "<style");
			else
				stringfrompos = strstr(tempraw, "<STYLE");

			positionStart = stringfrompos - tempraw;
		}

		if (((strstr(tempraw, "</style")) != 0) || (strstr(tempraw, "</STYLE") != 0)) // does 
		{
			if (strstr(tempraw, "<style") != 0)
				stringfrompos = strstr(tempraw, "</style");
			else
				stringfrompos = strstr(tempraw, "</STYLE");

			positionEnd = stringfrompos - tempraw;
		}
		BlockLength = (positionEnd - positionStart) + 8;

		if ((strstr(tempraw, "<style") != 0) && (strstr(tempraw, "</style") == 0))
			break;

		if ((strstr(tempraw, "<STYLE") != 0) && (strstr(tempraw, "</STYLE") == 0))
			break;

		///////////////////////////////////////
		if (((strstr(tempraw, "<style")) != 0) || ((strstr(tempraw, "<STYLE")) != 0))
			for (counter = 0; counter < BlockLength; counter++)
				tempraw[positionStart + counter] = ' ';

		if ((positionStart == 0) && (positionEnd == 0))
			break;
		if (positionStart > positionEnd)
			break;

		positionStart = 0;
		positionEnd = 0;
	}

	// 4//

	while (true) {
		Sleep(1); // avoid 100% CPU
		// /get start and end of block
		if (strstr(tempraw, "{") != 0) // does tag exist?
		{
			stringfrompos = strstr(tempraw, "{");
			positionStart = stringfrompos - tempraw;
		}

		if (strstr(tempraw, "}") != 0) // does tag exist?
		{
			stringfrompos = strstr(tempraw, "}");
			positionEnd = stringfrompos - tempraw;
		}
		BlockLength = (positionEnd - positionStart) + 1;

		if ((strstr(tempraw, "}") == 0) || (strstr(tempraw, "{") == 0))
			break;

		/////////////////////////////////////////
		if (strstr(tempraw, "{") != 0)
			for (counter = 0; counter < BlockLength; counter++)
				tempraw[positionStart + counter] = ' ';

		if ((positionStart == 0) && (positionEnd == 0))
			break;
		if (positionStart > positionEnd)
			break;

		positionStart = 0;
		positionEnd = 0;
	}

	// ///////////////////////////
	positionStart = 0;
	positionEnd = 0;

	mir_strncpy(truncated, tempraw, mir_strlen(truncated));
	free(tempraw);
}

/*****************************************************************************/
void EraseSymbols(char *truncated)
{
	int counter = 0;
	int position = 0, recpos = 0;
	char *stringfrompos;

	char *tempraw = (char*)malloc(MAXSIZE1);
	if (truncated)
		mir_strncpy(tempraw, truncated, MAXSIZE1);

	// //////
	while (true) {
		Sleep(1); // avoid 100% CPU

		/**/
		counter = 0;
		if ((strstr(tempraw, "&#")) != 0) {
			stringfrompos = strstr(tempraw, "&#");
			position = stringfrompos - tempraw;

			while (true) {
				tempraw[position + counter] = ' ';
				counter++;
				if (counter > 20)
					break;
				if (tempraw[position + counter] == ' ')
					break;
			}
			if (tempraw[position + counter] == ';')
				tempraw[position + counter] = ' ';
		}
		if (recpos == position)
			break;
		recpos = position;
	}

	mir_strncpy(truncated, tempraw, mir_strlen(truncated));
	free(tempraw);
}

/*****************************************************************************/
void NumSymbols(char *truncated)
{
	int counter = 0;
	int position = 0, recpos = 0;
	char*stringfrompos;
	char symbol[20];
	int character;

	char *tempraw = (char*)malloc(MAXSIZE1);
	if (truncated)
		mir_strncpy(tempraw, truncated, MAXSIZE1);

	while (true) {
		Sleep(1); // avoid 100% CPU

		counter = 0;

		if ((strstr(tempraw, "&#")) != 0) {
			stringfrompos = strstr(tempraw, "&#");
			position = stringfrompos - tempraw;

			while (true) {
				if (counter > 1)
					symbol[counter - 2] = tempraw[position + counter];

				tempraw[position + counter] = ' ';
				counter++;
				if (counter > 20)
					break;

				if ((tempraw[position + counter] == ';')) {
					symbol[counter - 2] = '\0';
					character = atoi(symbol);

					if (character > 0 && character < 256)
						memset(&tempraw[position], character, 1);
					break;
				}
			}

			if (tempraw[position + counter] == ';')
				tempraw[position + counter] = ' ';
		}
		if (recpos == position)
			break;
		recpos = position;
	}

	mir_strncpy(truncated, tempraw, mir_strlen(truncated));
	free(tempraw);
}

/*****************************************************************************/
void FastTagFilter(char *truncated)
{
	char *tempraw = (char*)malloc(MAXSIZE1);
	if (truncated)
		mir_strncpy(tempraw, truncated, MAXSIZE1);

	for (int counter = 0; counter < mir_strlen(tempraw); counter++) {
		if (tempraw[counter] == '<') {
			while (tempraw[counter] != '>') {
				if (counter >= mir_strlen(tempraw))
					break;

				tempraw[counter] = ' ';
				counter++;
			}
			if (tempraw[counter] == '>')
				tempraw[counter] = ' ';
		}
	}

	mir_strncpy(truncated, tempraw, mir_strlen(truncated));
	free(tempraw);
}

/*****************************************************************************/
void RemoveInvis(char *truncated, int AmountWspcRem)
{
	int erase = 0;
	int RemovalLevel = 0;

	char *tempraw = (char*)malloc(MAXSIZE1);
	if (truncated)
		mir_strncpy(tempraw, truncated, MAXSIZE1);

	switch (AmountWspcRem) {
	case 1:
		RemovalLevel = 80; // small
		break;
	case 2:
		RemovalLevel = 30; // medium
		break;
	case 3:
		RemovalLevel = 10; // large
		break;
	}

	for (int counter = 0; counter < mir_strlen(tempraw); counter++) {
		if (AmountWspcRem != 0 && AmountWspcRem != 4) {
			if ((tempraw[counter] == '\n') || (tempraw[counter] == ' ') || (tempraw[counter] == '\r'))
				erase = erase + 1;
			else
				erase = 0;

			if (erase > RemovalLevel)
				tempraw[counter] = ' ';
		}

		if (AmountWspcRem == 4)
			if ((tempraw[counter] == '\n') || (tempraw[counter] == ' ') || (tempraw[counter] == '\r'))
				tempraw[counter] = ' ';
	} // end for

	mir_strncpy(truncated, tempraw, mir_strlen(truncated));
	free(tempraw);
}

/*****************************************************************************/
void RemoveTabs(char *truncated)
{
	char *tempraw = (char*)malloc(MAXSIZE1);
	if (truncated)
		mir_strncpy(tempraw, truncated, MAXSIZE1);

	for (int counter = 0; counter < mir_strlen(tempraw); counter++) 
		if (tempraw[counter] == '\t')
			tempraw[counter] = ' ';

	mir_strncpy(truncated, tempraw, mir_strlen(truncated));
	free(tempraw);
}

/*****************************************************************************/
void Removewhitespace(char *truncated)
{
	int counter2 = 0;
	int pos1 = 0, pos2 = 0;

	for (int counter = 0; counter < mir_strlen(truncated); counter++) {
		if (truncated[counter] == ' ' && truncated[counter + 1] == ' ') {
			pos1 = counter + 1;
			counter2 = counter;

			while (truncated[counter2] == ' ')
				counter2++;

			pos2 = counter2;
			mir_strncpy(&truncated[pos1], &truncated[pos2], mir_strlen(&truncated[pos1]) - 1);
		} // end if
	} // end for
}

/*****************************************************************************/
void Filter(char *truncated)
{
	char tempraw[MAXSIZE1];
	mir_strncpy(tempraw, truncated, SIZEOF(tempraw));

	for (int counter = 0; counter < mir_strlen(tempraw); counter++)
		if ((tempraw[counter] == '\n') || (tempraw[counter] == '\r') || (tempraw[counter] == '\t'))
			mir_strncpy(&tempraw[counter], &tempraw[counter + 1], mir_strlen(&tempraw[counter]) - 1);

	mir_strncpy(truncated, tempraw, mir_strlen(truncated));
}
