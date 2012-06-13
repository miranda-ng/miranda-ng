/*
Popup Plus plugin for Miranda IM

Copyright	© 2002 Luca Santarelli,
			© 2005-2007 Victor Pavlychko
			© 2010 Merlin_de

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

===============================================================================

File name      : $HeadURL$
Revision       : $Revision$
Last change on : $Date$
Last change by : $Author$

===============================================================================
*/
	#define MODULNAME					"MTextControl"		//DB modul for this plugin
	#define MODULTITLE					"Text Display"

	
	#define MTEXT_DISPLAYNAME			"Miranda Text Control"
	#define MTEXT_FILENAME				"mtextcontrol.dll"
	
	#define __MAJOR_VERSION				0
	#define __MINOR_VERSION				8
	#define __RELEASE_NUM				0
	#define __BUILD_NUM					1

	#define __STRINGIFY(x)				#x
	#define __STRINGIFY2(x)				__STRINGIFY(x)
	#define __FILEVERSION_STRING		__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM
	#define __FILEVERSION_STRING_DOTS	__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM

	#define __VERSION_STRING			__STRINGIFY2(__FILEVERSION_STRING)
	#define __VERSION_STRING_DOT		__STRINGIFY2(__FILEVERSION_STRING_DOTS)

	#define MTEXT_DESCRIPTION	"Provides text render API for different plugins."
	#define MTEXT_AUTHOR		"Merlin_de, Victor Pavlychko"
	#define MTEXT_EMAIL			""
	#define MTEXT_COPYRIGHT		"©2005-2006 Victor Pavlychko, ©2010 Merlin_de"
	#define MTEXT_WEBPAGE		"http://www.miranda-im.org/"
	#define MTEXT_UUID			{0x69b9443b, 0xdc58, 0x4876, { 0xad, 0x39, 0xe3, 0xf4, 0x18, 0xa1, 0x33, 0xc5 } }
								//{69B9443B-DC58-4876-AD39-E3F418A133C5}
