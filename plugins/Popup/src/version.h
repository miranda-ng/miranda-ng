/*
Popup Plus plugin for Miranda IM

Copyright	� 2002 Luca Santarelli,
			� 2004-2007 Victor Pavlychko
			� 2010 MPK
			� 2010 Merlin_de

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
	#define PU_COMMONMODUL				"PopUp"				//common DB modul name for all Popup plugins
	#define PU_MODULNAME				"PopUpPlus"			//DB modul for this plugin
	#define PU_MODULCLASS				"PopUpCLASS"		//temp DB modul for this plugin
	#define MODULNAME					"PopUp"
	#define MODULNAME_LONG				"Popup Plus"		//dont use "PopUp Plus" coz notify register use "Popup Plus"
	#define MODULNAME_PLU				"PopUps"

	#define POPUP_FILENAME				"PopUp.dll"
	#define POPUP_DISPLAYNAME			"PopUp Plus"
	#define __FLVersionURL				"http://miranda-ng.org/"
	#define __FLVersionPrefix			"<span class=\"fileNameHeader\">PopUp Plus (Unicode) "
	#define __FLUpdateURL				"http://miranda-ng.org/distr/x32/Plugins/popup.zip"			//.zip
	#define __BetaUpdateURL				""

	#define __BetaVersionURL			"http://miranda-ng.org/"
	#define __BetaVersionPrefix			"beta: "
	#define __BetaChangelogURL			"http://miranda-ng.org/"

	#define __MAJOR_VERSION				2
	#define __MINOR_VERSION				1
	#define __RELEASE_NUM				1
	#define __BUILD_NUM					6

	#define __STRINGIFY(x)				#x
	#define __STRINGIFY2(x)				__STRINGIFY(x)
	#define __FILEVERSION_STRING		__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
	#define __FILEVERSION_STRING_DOTS	__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM

	#define __VERSION_STRING			__STRINGIFY2(__FILEVERSION_STRING)
	#define __VERSION_STRING_DOT		__STRINGIFY2(__FILEVERSION_STRING_DOTS)

	#define POPUP_DESCRIPTION	"Provides popup notification services for different plugins."
	#define POPUP_AUTHOR		"MPK, Merlin_de (Luca Santarelli, Victor Pavlychko)"
	#define POPUP_EMAIL			"mp-king@web.de"
	#define POPUP_COPYRIGHT		"�2002 Luca Santarelli, �2004-2007 Victor Pavlychko, �2010 MPK, Merlin_de"
	#define POPUP_WEBPAGE		"http://miranda-ng.org/"
	#define POPUP_UUID			{0x26a9125d, 0x7863, 0x4e01, {0xaf, 0xe, 0xd1, 0x4e, 0xf9, 0x5c, 0x50, 0x54}}
	#define POPUP_PID			0x12a3fdbdecd273ee67dbc4d6d00e9268

