// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2001-2004 Richard Hughes, Martin Öberg
// Copyright © 2004-2009 Joe Kucera, Bio
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
//
// -----------------------------------------------------------------------------
//  DESCRIPTION:
//
//  ChangeInfo Plugin stuff
//
// -----------------------------------------------------------------------------
#include "icqoscar.h"


static FieldNamesItem timezones[]={
  {24  ,LPGEN("GMT-12:00 Eniwetok; Kwajalein")},
  {23  ,LPGEN("GMT-11:30")},
  {22  ,LPGEN("GMT-11:00 Midway Island; Samoa")},
  {21  ,LPGEN("GMT-10:30")},
  {20  ,LPGEN("GMT-10:00 Hawaii")},
  {19  ,LPGEN("GMT-9:30")},
  {18  ,LPGEN("GMT-9:00 Alaska")},
  {17  ,LPGEN("GMT-8:30")},
  {16  ,LPGEN("GMT-8:00 Pacific Time; Tijuana")},
  {15  ,LPGEN("GMT-7:30")},
  {14  ,LPGEN("GMT-7:00 Arizona; Mountain Time")},
  {13  ,LPGEN("GMT-6:30")},
  {12  ,LPGEN("GMT-6:00 Central Time; Central America; Saskatchewan")},
  {11  ,LPGEN("GMT-5:30")},
  {10  ,LPGEN("GMT-5:00 Eastern Time; Bogota; Lima; Quito")},
  {9   ,LPGEN("GMT-4:30")},
  {8   ,LPGEN("GMT-4:00 Atlantic Time; Santiago; Caracas; La Paz")},
  {7   ,LPGEN("GMT-3:30 Newfoundland")},
  {6   ,LPGEN("GMT-3:00 Greenland; Buenos Aires; Georgetown")},
  {5   ,LPGEN("GMT-2:30")},
  {4   ,LPGEN("GMT-2:00 Mid-Atlantic")},
  {3   ,LPGEN("GMT-1:30")},
  {2   ,LPGEN("GMT-1:00 Cape Verde Islands; Azores")},
  {1   ,LPGEN("GMT-0:30")},
  {0   ,LPGEN("GMT+0:00 London; Dublin; Edinburgh; Lisbon; Casablanca")},
  {-1  ,LPGEN("GMT+0:30")},
  {-2  ,LPGEN("GMT+1:00 Central European Time; West Central Africa; Warsaw")},
  {-3  ,LPGEN("GMT+1:30")},
  {-4  ,LPGEN("GMT+2:00 Jerusalem; Helsinki; Harare; Cairo; Bucharest; Athens")},
  {-5  ,LPGEN("GMT+2:30")},
  {-6  ,LPGEN("GMT+3:00 Moscow; St. Petersburg; Nairobi; Kuwait; Baghdad")},
  {-7  ,LPGEN("GMT+3:30 Tehran")},
  {-8  ,LPGEN("GMT+4:00 Baku; Tbilisi; Yerevan; Abu Dhabi; Muscat")},
  {-9  ,LPGEN("GMT+4:30 Kabul")},
  {-10 ,LPGEN("GMT+5:00 Calcutta; Chennai; Mumbai; New Delhi; Ekaterinburg")},
  {-11 ,LPGEN("GMT+5:30")},
  {-12 ,LPGEN("GMT+6:00 Astana; Dhaka; Almaty; Novosibirsk; Sri Jayawardenepura")},
  {-13 ,LPGEN("GMT+6:30 Rangoon")},
  {-14 ,LPGEN("GMT+7:00 Bankok; Hanoi; Jakarta; Krasnoyarsk")},
  {-15 ,LPGEN("GMT+7:30")},
  {-16 ,LPGEN("GMT+8:00 Perth; Taipei; Singapore; Hong Kong; Beijing")},
  {-17 ,LPGEN("GMT+8:30")},
  {-18 ,LPGEN("GMT+9:00 Tokyo; Osaka; Seoul; Sapporo; Yakutsk")},
  {-19 ,LPGEN("GMT+9:30 Darwin; Adelaide")},
  {-20 ,LPGEN("GMT+10:00 East Australia; Guam; Vladivostok")},
  {-21 ,LPGEN("GMT+10:30")},
  {-22 ,LPGEN("GMT+11:00 Magadan; Solomon Is.; New Caledonia")},
  {-23 ,LPGEN("GMT+11:30")},
  {-24 ,LPGEN("GMT+12:00 Auckland; Wellington; Fiji; Kamchatka; Marshall Is.")},
  {-100,NULL}
};


static FieldNamesItem months[]={
  {1, LPGEN("January")},
  {2, LPGEN("February")},
  {3, LPGEN("March")},
  {4, LPGEN("April")},
  {5, LPGEN("May")},
  {6, LPGEN("June")},
  {7, LPGEN("July")},
  {8, LPGEN("August")},
  {9, LPGEN("September")},
  {10,LPGEN("October")},
  {11,LPGEN("November")},
  {12,LPGEN("December")},
  {0, NULL}
};


const int ageRange[]={13,0x7FFF};  // 14, 130
const int yearRange[]={1753,0x7FFF}; // 1880, 2000
const int dayRange[]={1,31};


const SettingItem setting[]={
  //personal
  {LPGEN("Personal"),       LI_DIVIDER},
  {LPGEN("Nickname"),       LI_STRING,     DBVT_UTF8,   "Nick"},
  {LPGEN("First name"),     LI_STRING,     DBVT_UTF8,   "FirstName"},
  {LPGEN("Last name"),      LI_STRING,     DBVT_UTF8,   "LastName"},
//  {LPGEN("Age"),            LI_NUMBER,     DBVT_WORD,   "Age",              ageRange},
  {LPGEN("Gender"),         LI_LIST,       DBVT_BYTE,   "Gender",           genderField},
  {LPGEN("About"),          LI_LONGSTRING, DBVT_UTF8,   "About"},
  //password
  {LPGEN("Password"),       LI_DIVIDER},
  {LPGEN("Password"),       LI_STRING|LIF_PASSWORD,DBVT_ASCIIZ, "Password"},
  //contact
  {LPGEN("Contact"),        LI_DIVIDER},
  {LPGEN("Primary e-mail"), LI_STRING,     DBVT_ASCIIZ, "e-mail0"},
  {LPGEN("Secondary e-mail"), LI_STRING,   DBVT_ASCIIZ, "e-mail1"},
  {LPGEN("Tertiary e-mail"), LI_STRING,    DBVT_ASCIIZ, "e-mail2"},
  {LPGEN("Homepage"),       LI_STRING,     DBVT_ASCIIZ, "Homepage"},
  {LPGEN("Street"),         LI_STRING,     DBVT_UTF8,   "Street"},
  {LPGEN("City"),           LI_STRING,     DBVT_UTF8,   "City"},
  {LPGEN("State"),          LI_STRING,     DBVT_UTF8,   "State"},
  {LPGEN("ZIP/postcode"),   LI_STRING,     DBVT_UTF8,   "ZIP"},
  {LPGEN("Country"),        LI_LIST,       DBVT_WORD,   "Country",          countryField},
  {LPGEN("Phone number"),   LI_STRING,     DBVT_ASCIIZ, "Phone"},
  {LPGEN("Fax number"),     LI_STRING,     DBVT_ASCIIZ, "Fax"},
  {LPGEN("Cellular number"),LI_STRING,     DBVT_ASCIIZ, "Cellular"},
  //more
  {LPGEN("Personal Detail"),LI_DIVIDER},
  {LPGEN("Timezone"),       LI_LIST|LIF_ZEROISVALID|LIF_SIGNED,DBVT_BYTE,   "Timezone", timezones},
  {LPGEN("Year of birth"),  LI_NUMBER,     DBVT_WORD,   "BirthYear",        yearRange},
  {LPGEN("Month of birth"), LI_LIST,       DBVT_BYTE,   "BirthMonth",       months},
  {LPGEN("Day of birth"),   LI_NUMBER,     DBVT_BYTE,   "BirthDay",         dayRange},
  {LPGEN("Marital Status"), LI_LIST,       DBVT_BYTE,   "MaritalStatus",    maritalField},
  {LPGEN("Spoken language 1"), LI_LIST,    DBVT_BYTE,   "Language1",        languageField},
  {LPGEN("Spoken language 2"), LI_LIST,    DBVT_BYTE,   "Language2",        languageField},
  {LPGEN("Spoken language 3"), LI_LIST,    DBVT_BYTE,   "Language3",        languageField},
  //more
  {LPGEN("Originally from"),LI_DIVIDER},
  {LPGEN("Street"),         LI_STRING,     DBVT_UTF8,   "OriginStreet"},
  {LPGEN("City"),           LI_STRING,     DBVT_UTF8,   "OriginCity"},
  {LPGEN("State"),          LI_STRING,     DBVT_UTF8,   "OriginState"},
  {LPGEN("Country"),        LI_LIST,       DBVT_WORD,   "OriginCountry",    countryField},
  //study
  {LPGEN("Education"),      LI_DIVIDER},
  {LPGEN("Level"),          LI_LIST,       DBVT_WORD,   "StudyLevel",       studyLevelField},
  {LPGEN("Institute"),      LI_STRING,     DBVT_UTF8,   "StudyInstitute"},
  {LPGEN("Degree"),         LI_STRING,     DBVT_UTF8,   "StudyDegree"},
  {LPGEN("Graduation Year"),LI_NUMBER,     DBVT_WORD,   "StudyYear",        yearRange},
  //work
  {LPGEN("Work"),           LI_DIVIDER},
  {LPGEN("Company name"),   LI_STRING,     DBVT_UTF8,   "Company"},
  {LPGEN("Company homepage"),LI_STRING,    DBVT_ASCIIZ, "CompanyHomepage"},
  {LPGEN("Company street"), LI_STRING,     DBVT_UTF8,   "CompanyStreet"},
  {LPGEN("Company city"),   LI_STRING,     DBVT_UTF8,   "CompanyCity"},
  {LPGEN("Company state"),  LI_STRING,     DBVT_UTF8,   "CompanyState"},
  {LPGEN("Company phone"),  LI_STRING,     DBVT_ASCIIZ, "CompanyPhone"},
  {LPGEN("Company fax"),    LI_STRING,     DBVT_ASCIIZ, "CompanyFax"},
  {LPGEN("Company ZIP/postcode"),LI_STRING,DBVT_UTF8,   "CompanyZIP"},
  {LPGEN("Company country"),LI_LIST,       DBVT_WORD,   "CompanyCountry",   countryField},
  {LPGEN("Company department"),LI_STRING,  DBVT_UTF8,   "CompanyDepartment"},
  {LPGEN("Company position"),LI_STRING,    DBVT_UTF8,   "CompanyPosition"},
  {LPGEN("Company industry"),LI_LIST,      DBVT_WORD,   "CompanyIndustry",  industryField},
//  {LPGEN("Company occupation"),LI_LIST,    DBVT_WORD,   "CompanyOccupation", occupationField},
  //interests
  {LPGEN("Personal Interests"),            LI_DIVIDER},
  {LPGEN("Interest category 1"),LI_LIST,   DBVT_WORD,   "Interest0Cat",     interestsField},
  {LPGEN("Interest areas 1"),LI_STRING,    DBVT_ASCIIZ, "Interest0Text"},
  {LPGEN("Interest category 2"),LI_LIST,   DBVT_WORD,   "Interest1Cat",     interestsField},
  {LPGEN("Interest areas 2"),LI_STRING,    DBVT_ASCIIZ, "Interest1Text"},
  {LPGEN("Interest category 3"),LI_LIST,   DBVT_WORD,   "Interest2Cat",     interestsField},
  {LPGEN("Interest areas 3"),LI_STRING,    DBVT_ASCIIZ, "Interest2Text"},
  {LPGEN("Interest category 4"),LI_LIST,   DBVT_WORD,   "Interest3Cat",     interestsField},
  {LPGEN("Interest areas 4"),LI_STRING,    DBVT_ASCIIZ, "Interest3Text"},
  //pastbackground
//  {LPGEN("Past Background"),               LI_DIVIDER},
//  {LPGEN("Category 1"),     LI_LIST,       DBVT_ASCIIZ, "Past0",            pastField},
//  {LPGEN("Past Background 1"),LI_STRING,   DBVT_ASCIIZ, "Past0Text"},
//  {LPGEN("Category 2"),     LI_LIST,       DBVT_ASCIIZ, "Past1",            pastField},
//  {LPGEN("Past Background 2"),LI_STRING,   DBVT_ASCIIZ, "Past1Text"},
//  {LPGEN("Category 3"),     LI_LIST,       DBVT_ASCIIZ, "Past2",            pastField},
//  {LPGEN("Past Background 3"),LI_STRING,   DBVT_ASCIIZ, "Past2Text"},
  //affiliation
//  {LPGEN("Affiliations"),                  LI_DIVIDER},
//  {LPGEN("Affiliation category 1"),LI_LIST,DBVT_ASCIIZ, "Affiliation0",     affiliationField},
//  {LPGEN("Affiliation 1"),  LI_STRING,     DBVT_ASCIIZ, "Affiliation0Text"},
//  {LPGEN("Affiliation category 2"),LI_LIST,DBVT_ASCIIZ, "Affiliation1",     affiliationField},
//  {LPGEN("Affiliation 2"),  LI_STRING,     DBVT_ASCIIZ, "Affiliation1Text"},
//  {LPGEN("Affiliation category 3"),LI_LIST,DBVT_ASCIIZ, "Affiliation2",     affiliationField},
//  {LPGEN("Affiliation 3"),  LI_STRING,     DBVT_ASCIIZ, "Affiliation2Text"}
};

const int settingCount = SIZEOF(setting);
