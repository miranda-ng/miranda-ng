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

#define Xpos_WIN_KEY     "win_Xpos"
#define Ypos_WIN_KEY     "win_Ypos"
#define BG_COLOR_KEY     "BgColor"
#define TXT_COLOR_KEY    "TxtColor"
#define WIN_HEIGHT_KEY   "Height"
#define SUPPRESS_ERR_KEY "Suppress error messages"
#define WIN_WIDTH_KEY    "Width"
#define FILE_KEY         "Filename"
#define APPEND_KEY       "Append"
#define UPDATE_ONSTART_KEY  "update_onboot"
#define UPDATE_ON_OPEN_KEY  "update_on_Window_open"
#define FONT_FACE_KEY    "FontFace"
#define FONT_BOLD_KEY    "FontBold"
#define FONT_ITALIC_KEY  "FontItalic"
#define FONT_UNDERLINE_KEY  "FontUnderline"
#define FONT_SIZE_KEY    "FontSize"
#define ERROR_POPUP_KEY  "UsePopupPlugin"
#define ENABLE_ALERTS_KEY "EnableAlerts"
#define ALERT_STRING_KEY "ALERTSTRING"
#define ALERT_TYPE_KEY   "Alert_Type"
#define APND_DATE_NAME_KEY  "AppendDateContact"
#define POP_DELAY_KEY    "PopUpDelay"
#define POP_BG_CLR_KEY   "PopBGClr"
#define POP_TXT_CLR_KEY  "PopTxtClr"
#define POP_USEWINCLRS_KEY  "PopUseWinClrs"
#define CONTACT_PREFIX_KEY  "PrefixDateContact"
#define USE_24_HOUR_KEY  "Use24hourformat"
#define POP_USESAMECLRS_KEY "PopUseSameClrs"
#define POP_USECUSTCLRS_KEY "PopUseCustomClrs"
#define LCLK_WINDOW_KEY  "LeftClkWindow"
#define LCLK_WEB_PGE_KEY "LeftClkWebPage"
#define LCLK_DISMISS_KEY "LeftClkDismiss"
#define RCLK_WINDOW_KEY  "RightClkWindow"
#define RCLK_WEB_PGE_KEY "RightClkWebPage"
#define RCLK_DISMISS_KEY "RightClkDismiss"
//#define INBUILTPOP_KEY   "UseInbuiltPopupAlert"

#define TIME  60
#define Def_color_bg     0x00ffffff
#define Def_color_txt    0x00000000
#define HK_SHOWHIDE      3001

#define MAXSIZE1         250000
#define MAXSIZE2         500000
#define MAXSIZE3         1000000

static UINT     expertOnlyControls[] =
{IDC_ADV_GRP,
   IDC_NO_PROTECT,
IDC_NOT_RECOMND_TXT};

//lets get rid of some warnings
void ErrorMsgs(char *contactname, char *displaytext);
void CodetoSymbol(char *truncated);
static void     GetData(void *hContact);
BOOL CALLBACK   DlgProcDisplayData(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
void FillFontListThread(HWND hwndDlg);
static BOOL CALLBACK DlgProcAlertOpt(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
int DataWndAlertCommand(void *AContact);


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
