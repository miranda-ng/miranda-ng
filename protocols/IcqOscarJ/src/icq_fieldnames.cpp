// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2009 Joe Kucera
// Copyright © 2012-2014 Miranda NG Team
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// -----------------------------------------------------------------------------

#include "stdafx.h"

FieldNamesItem timezonesField[] =
{
	{ -100, NULL }
};

const FieldNamesItem countryField[] = {
	{ 9999, LPGEN("Other") },
	{ 93, LPGEN("Afghanistan") },
	{ 355, LPGEN("Albania") },
	{ 213, LPGEN("Algeria") },
	{ 376, LPGEN("Andorra") },
	{ 244, LPGEN("Angola") },
	{ 1264, LPGEN("Anguilla") },
	{ 1268, LPGEN("Antigua and Barbuda") },
	{ 54, LPGEN("Argentina") },
	{ 374, LPGEN("Armenia") },
	{ 297, LPGEN("Aruba") },
	{ 247, LPGEN("Ascension Island") },
	{ 61, LPGEN("Australia") },
	{ 6720, LPGEN("Australia, Antarctic Territory") },	/* added country code 672(0)*/
	{ 614, LPGEN("Australia, Christmas Island") },	/* rename (from Christmas Island) and change to official county code 61(4) (from 672) */
	{ 61891, LPGEN("Australia, Cocos (Keeling) Islands") },	/* rename and change to official county code 61(891) (from 6102) */
	{ 6723, LPGEN("Australia, Norfolk Island") },	/* rename (from Norfolk Island) and change to official county code 672(3) (from 6722) */
	{ 43, LPGEN("Austria") },
	{ 994, LPGEN("Azerbaijan") },
	{ 1242, LPGEN("Bahamas") },
	{ 973, LPGEN("Bahrain") },
	{ 880, LPGEN("Bangladesh") },
	{ 1246, LPGEN("Barbados") },
	//{120, LPGEN("Barbuda")},   /* removed: it is not a country and no special island, see Antigua and Barbuda*/
	{ 375, LPGEN("Belarus") },
	{ 32, LPGEN("Belgium") },
	{ 501, LPGEN("Belize") },
	{ 229, LPGEN("Benin") },
	{ 1441, LPGEN("Bermuda") },
	{ 975, LPGEN("Bhutan") },
	{ 591, LPGEN("Bolivia") },
	{ 387, LPGEN("Bosnia and Herzegovina") },
	{ 267, LPGEN("Botswana") },
	{ 55, LPGEN("Brazil") },
	{ 106, LPGEN("British Virgin Islands") },
	{ 673, LPGEN("Brunei") },
	{ 359, LPGEN("Bulgaria") },
	{ 226, LPGEN("Burkina Faso") },
	{ 257, LPGEN("Burundi") },
	{ 855, LPGEN("Cambodia") },
	{ 237, LPGEN("Cameroon") },
	{ 1002, LPGEN("Canada") },
	{ 178, LPGEN("Canary Islands") },
	{ 238, LPGEN("Cape Verde") },
	{ 1345, LPGEN("Cayman Islands") },
	{ 236, LPGEN("Central African Republic") },
	{ 235, LPGEN("Chad") },
	{ 56, LPGEN("Chile") },
	{ 86, LPGEN("China") },
	{ 57, LPGEN("Colombia") },
	{ 269, LPGEN("Comoros") },
	{ 243, LPGEN("Congo, Democratic Republic of the") },
	{ 242, LPGEN("Congo, Republic of the") },
	{ 682, LPGEN("Cook Islands") },
	{ 506, LPGEN("Costa Rica") },
	{ 225, LPGEN("Cote d'Ivoire") },
	{ 385, LPGEN("Croatia") },
	{ 53, LPGEN("Cuba") },
	{ 357, LPGEN("Greek, Republic of South Cyprus") },  /* rename coz Turkey, Republic of Northern Cyprus */
	{ 420, LPGEN("Czech Republic") },
	{ 45, LPGEN("Denmark") },
	{ 246, LPGEN("Diego Garcia") },
	{ 253, LPGEN("Djibouti") },
	{ 1767, LPGEN("Dominica") },
	{ 1809, LPGEN("Dominican Republic") },
	{ 593, LPGEN("Ecuador") },
	{ 20, LPGEN("Egypt") },
	{ 503, LPGEN("El Salvador") },
	{ 240, LPGEN("Equatorial Guinea") },
	{ 291, LPGEN("Eritrea") },
	{ 372, LPGEN("Estonia") },
	{ 251, LPGEN("Ethiopia") },
	{ 3883, LPGEN("Europe") },	/* add county code  +388 3 official European Telephony Numbering Space*/
	{ 298, LPGEN("Faroe Islands") },
	{ 500, LPGEN("Falkland Islands") },
	{ 679, LPGEN("Fiji") },
	{ 358, LPGEN("Finland") },
	{ 33, LPGEN("France") },
	{ 5901, LPGEN("French Antilles") },
	{ 594, LPGEN("French Guiana") },
	{ 689, LPGEN("French Polynesia") },
	{ 241, LPGEN("Gabon") },
	{ 220, LPGEN("Gambia") },
	{ 995, LPGEN("Georgia") },
	{ 49, LPGEN("Germany") },
	{ 233, LPGEN("Ghana") },
	{ 350, LPGEN("Gibraltar") },
	{ 30, LPGEN("Greece") },
	{ 299, LPGEN("Greenland") },
	{ 1473, LPGEN("Grenada") },
	{ 590, LPGEN("Guadeloupe") },
	{ 1671, LPGEN("Guam, US Territory of") },
	{ 502, LPGEN("Guatemala") },
	{ 224, LPGEN("Guinea") },
	{ 245, LPGEN("Guinea-Bissau") },
	{ 592, LPGEN("Guyana") },
	{ 509, LPGEN("Haiti") },
	{ 504, LPGEN("Honduras") },
	{ 852, LPGEN("Hong Kong") },
	{ 36, LPGEN("Hungary") },
	{ 354, LPGEN("Iceland") },
	{ 91, LPGEN("India") },
	{ 62, LPGEN("Indonesia") },
	{ 98, LPGEN("Iran") },
	{ 964, LPGEN("Iraq") },
	{ 353, LPGEN("Ireland") },
	{ 972, LPGEN("Israel") },
	{ 39, LPGEN("Italy") },
	{ 1876, LPGEN("Jamaica") },
	{ 81, LPGEN("Japan") },
	{ 962, LPGEN("Jordan") },
	{ 705, LPGEN("Kazakhstan") },
	{ 254, LPGEN("Kenya") },
	{ 686, LPGEN("Kiribati") },
	{ 850, LPGEN("Korea, North") },
	{ 82, LPGEN("Korea, South") },
	{ 965, LPGEN("Kuwait") },
	{ 996, LPGEN("Kyrgyzstan") },
	{ 856, LPGEN("Laos") },
	{ 371, LPGEN("Latvia") },
	{ 961, LPGEN("Lebanon") },
	{ 266, LPGEN("Lesotho") },
	{ 231, LPGEN("Liberia") },
	{ 218, LPGEN("Libya") },
	{ 423, LPGEN("Liechtenstein") },
	{ 370, LPGEN("Lithuania") },
	{ 352, LPGEN("Luxembourg") },
	{ 853, LPGEN("Macau") },
	{ 389, LPGEN("Macedonia, Republic of") },
	{ 261, LPGEN("Madagascar") },
	{ 265, LPGEN("Malawi") },
	{ 60, LPGEN("Malaysia") },
	{ 960, LPGEN("Maldives") },
	{ 223, LPGEN("Mali") },
	{ 356, LPGEN("Malta") },
	{ 692, LPGEN("Marshall Islands") },
	{ 596, LPGEN("Martinique") },
	{ 222, LPGEN("Mauritania") },
	{ 230, LPGEN("Mauritius") },
	{ 262, LPGEN("Mayotte") },
	{ 52, LPGEN("Mexico") },
	{ 691, LPGEN("Micronesia, Federated States of") },
	{ 373, LPGEN("Moldova") },
	{ 377, LPGEN("Monaco") },
	{ 976, LPGEN("Mongolia") },
	{ 1664, LPGEN("Montserrat") },
	{ 212, LPGEN("Morocco") },
	{ 258, LPGEN("Mozambique") },
	{ 95, LPGEN("Myanmar") },
	{ 264, LPGEN("Namibia") },
	{ 674, LPGEN("Nauru") },
	{ 977, LPGEN("Nepal") },
	{ 31, LPGEN("Netherlands") },
	{ 599, LPGEN("Netherlands Antilles") },  /* dissolved 2010 */
	{ 5995, LPGEN("St. Maarten") },	/* add new country in 2010 (from Netherlands Antilles) */
	{ 5999, LPGEN("Curacao") },	/* add new country in 2010 (from Netherlands Antilles) */
	{ 5997, LPGEN("Netherlands (Bonaire Island)") },	/* add new Part of Netherlands in 2010 (from Netherlands Antilles) */
	{ 59946, LPGEN("Netherlands (Saba Island)") },	/* add new Part of Netherlands in 2010 (from Netherlands Antilles) */
	{ 59938, LPGEN("Netherlands (St. Eustatius Island)") },	/* add new Part of Netherlands in 2010 (from Netherlands Antilles) */
	{ 687, LPGEN("New Caledonia") },
	{ 64, LPGEN("New Zealand") },
	{ 505, LPGEN("Nicaragua") },
	{ 227, LPGEN("Niger") },
	{ 234, LPGEN("Nigeria") },
	{ 683, LPGEN("Niue") },
	{ 1670, LPGEN("Northern Mariana Islands, US Territory of") },	/* added NANP */
	{ 47, LPGEN("Norway") },
	{ 968, LPGEN("Oman") },
	{ 92, LPGEN("Pakistan") },
	{ 680, LPGEN("Palau") },
	{ 507, LPGEN("Panama") },
	{ 675, LPGEN("Papua New Guinea") },
	{ 595, LPGEN("Paraguay") },
	{ 51, LPGEN("Peru") },
	{ 63, LPGEN("Philippines") },
	{ 48, LPGEN("Poland") },
	{ 351, LPGEN("Portugal") },
	{ 1939, LPGEN("Puerto Rico") },
	{ 974, LPGEN("Qatar") },
	{ 262, LPGEN("Reunion") },
	{ 40, LPGEN("Romania") },
	{ 7, LPGEN("Russia") },
	{ 250, LPGEN("Rwanda") },
	{ 1684, LPGEN("Samoa (USA)") },  /* rename (from American Samoa) change county code to NANP (from 684) */
	{ 685, LPGEN("Samoa, Western") },  /* rename (from Western Samoa) */
	{ 290, LPGEN("Saint Helena") },
	{ 1869, LPGEN("Saint Kitts and Nevis") },
	{ 1758, LPGEN("Saint Lucia") },
	{ 508, LPGEN("Saint Pierre and Miquelon") },
	{ 1784, LPGEN("Saint Vincent and the Grenadines") },
	{ 378, LPGEN("San Marino") },
	{ 239, LPGEN("Sao Tome and Principe") },
	{ 966, LPGEN("Saudi Arabia") },
	{ 442, LPGEN("Scotland") },
	{ 221, LPGEN("Senegal") },
	{ 248, LPGEN("Seychelles") },
	{ 232, LPGEN("Sierra Leone") },
	{ 65, LPGEN("Singapore") },
	{ 421, LPGEN("Slovakia") },
	{ 386, LPGEN("Slovenia") },
	{ 677, LPGEN("Solomon Islands") },
	{ 252, LPGEN("Somalia") },
	{ 27, LPGEN("South Africa") },
	{ 34, LPGEN("Spain") },
	{ 3492, LPGEN("Spain, Canary Islands") },	/*rename and change county code to 34(92) spain + canary code*/
	{ 94, LPGEN("Sri Lanka") },
	{ 249, LPGEN("Sudan") },
	{ 597, LPGEN("Suriname") },
	{ 268, LPGEN("Swaziland") },
	{ 46, LPGEN("Sweden") },
	{ 41, LPGEN("Switzerland") },
	{ 963, LPGEN("Syria") },
	{ 886, LPGEN("Taiwan") },
	{ 992, LPGEN("Tajikistan") },
	{ 255, LPGEN("Tanzania") },
	{ 66, LPGEN("Thailand") },
	//{6702, LPGEN("Tinian Island")},   /* removed: it is not a country it is part of Northern Mariana Islands, US Territory of */
	{ 670, LPGEN("Timor, East") },	/* added (is part off Northern Mariana Islands but not US Territory*/
	{ 228, LPGEN("Togo") },
	{ 690, LPGEN("Tokelau") },
	{ 676, LPGEN("Tonga") },
	{ 1868, LPGEN("Trinidad and Tobago") },
	{ 216, LPGEN("Tunisia") },
	{ 90, LPGEN("Turkey") },
	{ 90392, LPGEN("Turkey, Republic of Northern Cyprus") },	/* added (is diffrent from Greek part)*/
	{ 993, LPGEN("Turkmenistan") },
	{ 1649, LPGEN("Turks and Caicos Islands") },
	{ 688, LPGEN("Tuvalu") },
	{ 256, LPGEN("Uganda") },
	{ 380, LPGEN("Ukraine") },
	{ 971, LPGEN("United Arab Emirates") },
	{ 44, LPGEN("United Kingdom") },
	{ 598, LPGEN("Uruguay") },
	{ 1, LPGEN("USA") },
	{ 998, LPGEN("Uzbekistan") },
	{ 678, LPGEN("Vanuatu") },
	{ 379, LPGEN("Vatican City") },
	{ 58, LPGEN("Venezuela") },
	{ 84, LPGEN("Vietnam") },
	{ 1284, LPGEN("Virgin Islands (UK)") },	/* change county code to NANP (from 105) - rename coz Virgin Islands (USA) */
	{ 1340, LPGEN("Virgin Islands (USA)") },	/* change county code to NANP (from 123) */
	{ 441, LPGEN("Wales") },
	{ 681, LPGEN("Wallis and Futuna") },
	{ 967, LPGEN("Yemen") },
	{ 38, LPGEN("Yugoslavia") },
	{ 381, LPGEN("Serbia") },	/* rename need (from Yugoslavia)*/
	{ 383, LPGEN("Kosovo, Republic of") },	/*change country code (from 3811),  rename need (from Yugoslavia - Serbia) */
	{ 382, LPGEN("Montenegro") },	/* rename need (from Yugoslavia - Montenegro) */
	{ 260, LPGEN("Zambia") },
	{ 263, LPGEN("Zimbabwe") },
	{ 0, NULL }
};

const FieldNamesItem interestsField[] =
{
	{ 137, LPGEN("50's") },
	{ 134, LPGEN("60's") },
	{ 135, LPGEN("70's") },
	{ 136, LPGEN("80's") },
	{ 100, LPGEN("Art") },
	{ 128, LPGEN("Astronomy") },
	{ 147, LPGEN("Audio and Visual") },
	{ 125, LPGEN("Business") },
	{ 146, LPGEN("Business Services") },
	{ 101, LPGEN("Cars") },
	{ 102, LPGEN("Celebrity Fans") },
	{ 130, LPGEN("Clothing") },
	{ 103, LPGEN("Collections") },
	{ 104, LPGEN("Computers") },
	{ 105, LPGEN("Culture") },
	{ 122, LPGEN("Ecology") },
	{ 139, LPGEN("Entertainment") },
	{ 138, LPGEN("Finance and Corporate") },
	{ 106, LPGEN("Fitness") },
	{ 142, LPGEN("Health and Beauty") },
	{ 108, LPGEN("Hobbies") },
	{ 150, LPGEN("Home Automation") },
	{ 144, LPGEN("Household Products") },
	{ 107, LPGEN("Games") },
	{ 124, LPGEN("Government") },
	{ 109, LPGEN("ICQ - Help") },
	{ 110, LPGEN("Internet") },
	{ 111, LPGEN("Lifestyle") },
	{ 145, LPGEN("Mail Order Catalog") },
	{ 143, LPGEN("Media") },
	{ 112, LPGEN("Movies and TV") },
	{ 113, LPGEN("Music") },
	{ 126, LPGEN("Mystics") },
	{ 123, LPGEN("News and Media") },
	{ 114, LPGEN("Outdoors") },
	{ 115, LPGEN("Parenting") },
	{ 131, LPGEN("Parties") },
	{ 116, LPGEN("Pets and Animals") },
	{ 149, LPGEN("Publishing") },
	{ 117, LPGEN("Religion") },
	{ 141, LPGEN("Retail Stores") },
	{ 118, LPGEN("Science") },
	{ 119, LPGEN("Skills") },
	{ 133, LPGEN("Social science") },
	{ 129, LPGEN("Space") },
	{ 148, LPGEN("Sporting and Athletic") },
	{ 120, LPGEN("Sports") },
	{ 127, LPGEN("Travel") },
	{ 121, LPGEN("Web Design") },
	{ 132, LPGEN("Women") },
	{ -1, NULL }
};

const FieldNamesItem languageField[] =
{
	{ 55, LPGEN("Afrikaans") },
	{ 58, LPGEN("Albanian") },
	{ 1, LPGEN("Arabic") },
	{ 59, LPGEN("Armenian") },
	{ 68, LPGEN("Azerbaijani") },
	{ 72, LPGEN("Belorussian") },
	{ 2, LPGEN("Bhojpuri") },
	{ 56, LPGEN("Bosnian") },
	{ 3, LPGEN("Bulgarian") },
	{ 4, LPGEN("Burmese") },
	{ 5, LPGEN("Cantonese") },
	{ 6, LPGEN("Catalan") },
	{ 61, LPGEN("Chamorro") },
	{ 7, LPGEN("Chinese") },
	{ 8, LPGEN("Croatian") },
	{ 9, LPGEN("Czech") },
	{ 10, LPGEN("Danish") },
	{ 11, LPGEN("Dutch") },
	{ 12, LPGEN("English") },
	{ 13, LPGEN("Esperanto") },
	{ 14, LPGEN("Estonian") },
	{ 15, LPGEN("Farsi") },
	{ 16, LPGEN("Finnish") },
	{ 17, LPGEN("French") },
	{ 18, LPGEN("Gaelic") },
	{ 19, LPGEN("German") },
	{ 20, LPGEN("Greek") },
	{ 70, LPGEN("Gujarati") },
	{ 21, LPGEN("Hebrew") },
	{ 22, LPGEN("Hindi") },
	{ 23, LPGEN("Hungarian") },
	{ 24, LPGEN("Icelandic") },
	{ 25, LPGEN("Indonesian") },
	{ 26, LPGEN("Italian") },
	{ 27, LPGEN("Japanese") },
	{ 28, LPGEN("Khmer") },
	{ 29, LPGEN("Korean") },
	{ 69, LPGEN("Kurdish") },
	{ 30, LPGEN("Lao") },
	{ 31, LPGEN("Latvian") },
	{ 32, LPGEN("Lithuanian") },
	{ 65, LPGEN("Macedonian") },
	{ 33, LPGEN("Malay") },
	{ 63, LPGEN("Mandarin") },
	{ 62, LPGEN("Mongolian") },
	{ 34, LPGEN("Norwegian") },
	{ 57, LPGEN("Persian") },
	{ 35, LPGEN("Polish") },
	{ 36, LPGEN("Portuguese") },
	{ 60, LPGEN("Punjabi") },
	{ 37, LPGEN("Romanian") },
	{ 38, LPGEN("Russian") },
	{ 39, LPGEN("Serbian") },
	{ 66, LPGEN("Sindhi") },
	{ 40, LPGEN("Slovak") },
	{ 41, LPGEN("Slovenian") },
	{ 42, LPGEN("Somali") },
	{ 43, LPGEN("Spanish") },
	{ 44, LPGEN("Swahili") },
	{ 45, LPGEN("Swedish") },
	{ 46, LPGEN("Tagalog") },
	{ 64, LPGEN("Taiwanese") },
	{ 71, LPGEN("Tamil") },
	{ 47, LPGEN("Tatar") },
	{ 48, LPGEN("Thai") },
	{ 49, LPGEN("Turkish") },
	{ 50, LPGEN("Ukrainian") },
	{ 51, LPGEN("Urdu") },
	{ 52, LPGEN("Vietnamese") },
	{ 67, LPGEN("Welsh") },
	{ 53, LPGEN("Yiddish") },
	{ 54, LPGEN("Yoruba") },
	{ 0, NULL }
};

const FieldNamesItem pastField[] =
{
	{ 300, LPGEN("Elementary School") },
	{ 301, LPGEN("High School") },
	{ 302, LPGEN("College") },
	{ 303, LPGEN("University") },
	{ 304, LPGEN("Military") },
	{ 305, LPGEN("Past Work Place") },
	{ 306, LPGEN("Past Organization") },
	{ 399, LPGEN("Other") },
	{ 0, NULL }
};

const FieldNamesItem genderField[] =
{
	{ 'F', LPGEN("Female") },
	{ 'M', LPGEN("Male") },
	{ 0, NULL }
};

const FieldNamesItem studyLevelField[] =
{
	{ 4, LPGEN("Associated degree") },
	{ 5, LPGEN("Bachelor's degree") },
	{ 1, LPGEN("Elementary") },
	{ 2, LPGEN("High-school") },
	{ 6, LPGEN("Master's degree") },
	{ 7, LPGEN("PhD") },
	{ 8, LPGEN("Postdoctoral") },
	{ 3, LPGEN("University / College") },
	{ 0, NULL }
};

const FieldNamesItem industryField[] =
{
	{ 2, LPGEN("Agriculture") },
	{ 3, LPGEN("Arts") },
	{ 4, LPGEN("Construction") },
	{ 5, LPGEN("Consumer Goods") },
	{ 6, LPGEN("Corporate Services") },
	{ 7, LPGEN("Education") },
	{ 8, LPGEN("Finance") },
	{ 9, LPGEN("Government") },
	{ 10, LPGEN("High Tech") },
	{ 11, LPGEN("Legal") },
	{ 12, LPGEN("Manufacturing") },
	{ 13, LPGEN("Media") },
	{ 14, LPGEN("Medical & Health Care") },
	{ 15, LPGEN("Non-Profit Organization Management") },
	{ 19, LPGEN("Other") },
	{ 16, LPGEN("Recreation, Travel & Entertainment") },
	{ 17, LPGEN("Service Industry") },
	{ 18, LPGEN("Transportation") },
	{ 0, NULL }
};

const FieldNamesItem occupationField[] =
{
	{ 1, LPGEN("Academic") },
	{ 2, LPGEN("Administrative") },
	{ 3, LPGEN("Art/Entertainment") },
	{ 4, LPGEN("College Student") },
	{ 5, LPGEN("Computers") },
	{ 6, LPGEN("Community & Social") },
	{ 7, LPGEN("Education") },
	{ 8, LPGEN("Engineering") },
	{ 9, LPGEN("Financial Services") },
	{ 10, LPGEN("Government") },
	{ 11, LPGEN("High School Student") },
	{ 12, LPGEN("Home") },
	{ 13, LPGEN("ICQ - Providing Help") },
	{ 14, LPGEN("Law") },
	{ 15, LPGEN("Managerial") },
	{ 16, LPGEN("Manufacturing") },
	{ 17, LPGEN("Medical/Health") },
	{ 18, LPGEN("Military") },
	{ 19, LPGEN("Non-Government Organization") },
	{ 20, LPGEN("Professional") },
	{ 21, LPGEN("Retail") },
	{ 22, LPGEN("Retired") },
	{ 23, LPGEN("Science & Research") },
	{ 24, LPGEN("Sports") },
	{ 25, LPGEN("Technical") },
	{ 26, LPGEN("University Student") },
	{ 27, LPGEN("Web Building") },
	{ 99, LPGEN("Other Services") },
	{ 0, NULL }
};

const FieldNamesItem affiliationField[] =
{
	{ 200, LPGEN("Alumni Org.") },
	{ 201, LPGEN("Charity Org.") },
	{ 202, LPGEN("Club/Social Org.") },
	{ 203, LPGEN("Community Org.") },
	{ 204, LPGEN("Cultural Org.") },
	{ 205, LPGEN("Fan Clubs") },
	{ 206, LPGEN("Fraternity/Sorority") },
	{ 207, LPGEN("Hobbyists Org.") },
	{ 208, LPGEN("International Org.") },
	{ 209, LPGEN("Nature and Environment Org.") },
	{ 210, LPGEN("Professional Org.") },
	{ 211, LPGEN("Scientific/Technical Org.") },
	{ 212, LPGEN("Self Improvement Group") },
	{ 213, LPGEN("Spiritual/Religious Org.") },
	{ 214, LPGEN("Sports Org.") },
	{ 215, LPGEN("Support Org.") },
	{ 216, LPGEN("Trade and Business Org.") },
	{ 217, LPGEN("Union") },
	{ 218, LPGEN("Volunteer Org.") },
	{ 299, LPGEN("Other") },
	{ 0, NULL }
};

const FieldNamesItem agesField[] =
{
	{ 0x0011000D, LPGEN("13-17") },
	{ 0x00160012, LPGEN("18-22") },
	{ 0x001D0017, LPGEN("23-29") },
	{ 0x0027001E, LPGEN("30-39") },
	{ 0x00310028, LPGEN("40-49") },
	{ 0x003B0032, LPGEN("50-59") },
	{ 0x2710003C, LPGEN("60-above") },
	{ -1, NULL }
};

const FieldNamesItem maritalField[] =
{
	{ 10, LPGEN("Single") },
	{ 11, LPGEN("Close relationships") },
	{ 12, LPGEN("Engaged") },
	{ 20, LPGEN("Married") },
	{ 30, LPGEN("Divorced") },
	{ 31, LPGEN("Separated") },
	{ 40, LPGEN("Widowed") },
	{ 50, LPGEN("Open relationship") },
	{ 255, LPGEN("Other") },
	{ 0, NULL }
};

char* LookupFieldName(const FieldNamesItem *table, int code)
{
	if (code != 0)
		for (int i = 0; table[i].text; i++)
			if (table[i].code == code)
				return table[i].text;

	return NULL;
}

char* LookupFieldNameUtf(const FieldNamesItem *table, int code, char *str, size_t strsize)
{
	char *szText = LookupFieldName(table, code);
	if (szText)
		return ICQTranslateUtfStatic(szText, str, strsize);

	return NULL;
}
