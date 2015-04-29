// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2001-2004 Richard Hughes, Martin Öberg
// Copyright © 2004-2009 Joe Kucera, Bio
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
//  DESCRIPTION:
//
//  ChangeInfo Plugin stuff
// -----------------------------------------------------------------------------

#include "stdafx.h"

static FieldNamesItem months[] = {
	{ 1, LPGEN("January") },
	{ 2, LPGEN("February") },
	{ 3, LPGEN("March") },
	{ 4, LPGEN("April") },
	{ 5, LPGEN("May") },
	{ 6, LPGEN("June") },
	{ 7, LPGEN("July") },
	{ 8, LPGEN("August") },
	{ 9, LPGEN("September") },
	{ 10, LPGEN("October") },
	{ 11, LPGEN("November") },
	{ 12, LPGEN("December") },
	{ 0, NULL }
};

const int ageRange[] = { 13, 0x7FFF };  // 14, 130
const int yearRange[] = { 1753, 0x7FFF }; // 1880, 2000
const int dayRange[] = { 1, 31 };

const SettingItem setting[] =
{
	// personal
	{ LPGEN("Personal"), LI_DIVIDER },
	{ LPGEN("Nickname"), LI_STRING, DBVT_UTF8, "Nick" },
	{ LPGEN("First name"), LI_STRING, DBVT_UTF8, "FirstName" },
	{ LPGEN("Last name"), LI_STRING, DBVT_UTF8, "LastName" },
	{ LPGEN("Gender"), LI_LIST, DBVT_BYTE, "Gender", genderField },
	{ LPGEN("About"), LI_LONGSTRING, DBVT_UTF8, "About" },

	// password
	{ LPGEN("Password"), LI_DIVIDER },
	{ LPGEN("Password"), LI_STRING | LIF_PASSWORD, DBVT_UTF8, "Password" },

	// contact
	{ LPGEN("Contact"), LI_DIVIDER },
	{ LPGEN("Primary e-mail"), LI_STRING, DBVT_UTF8, "e-mail0" },
	{ LPGEN("Secondary e-mail"), LI_STRING, DBVT_UTF8, "e-mail1" },
	{ LPGEN("Tertiary e-mail"), LI_STRING, DBVT_UTF8, "e-mail2" },
	{ LPGEN("Homepage"), LI_STRING, DBVT_UTF8, "Homepage" },
	{ LPGEN("Street"), LI_STRING, DBVT_UTF8, "Street" },
	{ LPGEN("City"), LI_STRING, DBVT_UTF8, "City" },
	{ LPGEN("State"), LI_STRING, DBVT_UTF8, "State" },
	{ LPGEN("ZIP/postcode"), LI_STRING, DBVT_UTF8, "ZIP" },
	{ LPGEN("Country"), LI_LIST, DBVT_WORD, "Country", countryField },
	{ LPGEN("Phone number"), LI_STRING, DBVT_UTF8, "Phone" },
	{ LPGEN("Fax number"), LI_STRING, DBVT_UTF8, "Fax" },
	{ LPGEN("Cellular number"), LI_STRING, DBVT_UTF8, "Cellular" },
	
	// more
	{ LPGEN("Personal Detail"), LI_DIVIDER },
	{ LPGEN("Timezone"), LI_LIST | LIF_ZEROISVALID | LIF_SIGNED, DBVT_BYTE, "Timezone", timezonesField },
	{ LPGEN("Year of birth"), LI_NUMBER, DBVT_WORD, "BirthYear", yearRange },
	{ LPGEN("Month of birth"), LI_LIST, DBVT_BYTE, "BirthMonth", months },
	{ LPGEN("Day of birth"), LI_NUMBER, DBVT_BYTE, "BirthDay", dayRange },
	{ LPGEN("Marital Status"), LI_LIST, DBVT_BYTE, "MaritalStatus", maritalField },
	{ LPGEN("Spoken language 1"), LI_LIST, DBVT_BYTE, "Language1", languageField },
	{ LPGEN("Spoken language 2"), LI_LIST, DBVT_BYTE, "Language2", languageField },
	{ LPGEN("Spoken language 3"), LI_LIST, DBVT_BYTE, "Language3", languageField },

	// more
	{ LPGEN("Originally from"), LI_DIVIDER },
	{ LPGEN("Street"), LI_STRING, DBVT_UTF8, "OriginStreet" },
	{ LPGEN("City"), LI_STRING, DBVT_UTF8, "OriginCity" },
	{ LPGEN("State"), LI_STRING, DBVT_UTF8, "OriginState" },
	{ LPGEN("Country"), LI_LIST, DBVT_WORD, "OriginCountry", countryField },

	// study
	{ LPGEN("Education"), LI_DIVIDER },
	{ LPGEN("Level"), LI_LIST, DBVT_WORD, "StudyLevel", studyLevelField },
	{ LPGEN("Institute"), LI_STRING, DBVT_UTF8, "StudyInstitute" },
	{ LPGEN("Degree"), LI_STRING, DBVT_UTF8, "StudyDegree" },
	{ LPGEN("Graduation Year"), LI_NUMBER, DBVT_WORD, "StudyYear", yearRange },

	// work
	{ LPGEN("Work"), LI_DIVIDER },
	{ LPGEN("Company name"), LI_STRING, DBVT_UTF8, "Company" },
	{ LPGEN("Company homepage"), LI_STRING, DBVT_UTF8, "CompanyHomepage" },
	{ LPGEN("Company street"), LI_STRING, DBVT_UTF8, "CompanyStreet" },
	{ LPGEN("Company city"), LI_STRING, DBVT_UTF8, "CompanyCity" },
	{ LPGEN("Company state"), LI_STRING, DBVT_UTF8, "CompanyState" },
	{ LPGEN("Company phone"), LI_STRING, DBVT_UTF8, "CompanyPhone" },
	{ LPGEN("Company fax"), LI_STRING, DBVT_UTF8, "CompanyFax" },
	{ LPGEN("Company ZIP/postcode"), LI_STRING, DBVT_UTF8, "CompanyZIP" },
	{ LPGEN("Company country"), LI_LIST, DBVT_WORD, "CompanyCountry", countryField },
	{ LPGEN("Company department"), LI_STRING, DBVT_UTF8, "CompanyDepartment" },
	{ LPGEN("Company position"), LI_STRING, DBVT_UTF8, "CompanyPosition" },
	{ LPGEN("Company industry"), LI_LIST, DBVT_WORD, "CompanyIndustry", industryField },

	//interests
	{ LPGEN("Personal Interests"), LI_DIVIDER },
	{ LPGEN("Interest category 1"), LI_LIST, DBVT_WORD, "Interest0Cat", interestsField },
	{ LPGEN("Interest areas 1"), LI_STRING, DBVT_UTF8, "Interest0Text" },
	{ LPGEN("Interest category 2"), LI_LIST, DBVT_WORD, "Interest1Cat", interestsField },
	{ LPGEN("Interest areas 2"), LI_STRING, DBVT_UTF8, "Interest1Text" },
	{ LPGEN("Interest category 3"), LI_LIST, DBVT_WORD, "Interest2Cat", interestsField },
	{ LPGEN("Interest areas 3"), LI_STRING, DBVT_UTF8, "Interest2Text" },
	{ LPGEN("Interest category 4"), LI_LIST, DBVT_WORD, "Interest3Cat", interestsField },
	{ LPGEN("Interest areas 4"), LI_STRING, DBVT_UTF8, "Interest3Text" },
};

const int settingCount = SIZEOF(setting);
