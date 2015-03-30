/* ============================================================================
Hardware HotKeys plugin for Miranda NG.
Copyright © Eugene f2065, http://f2065.narod.ru, f2065 mail.ru, ICQ 35078112

This program is free software; you can redistribute it and / or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111 - 1307, USA.
============================================================================ */

// история версий:
// 1.0.0.6 - первая версия Hardware HotKeys для Miranda NG (более ранние версии были для Miranda IM), исправление старых ошибок.


#define __MAJOR_VERSION          1
#define __MINOR_VERSION          0
#define __RELEASE_NUM            0
#define __BUILD_NUM              6

#include <stdver.h>

#define __PLUGIN_NAME      "Hardware HotKeys" // dll-fileinfo "FileVersion" и "ProductName", меню в настройках миранды, название плагина в миранде. Нелокализуемое!
#define __FILENAME         "HwHotKeys.dll" // dll-fileinfo "OriginalFilename"
#define __DESCRIPTION_MIR  "Hardware HotKeys plugin for Miranda NG\r\nAllows to assign expanded multimedia keys (only for PS/2-keyboards)." // описание плагина в миранде (локализуемое)
#define __DESCRIPTION_DLL  "Hardware HotKeys plugin for Miranda NG." // описание плагина в dll-fileinfo "FileDescription" (там многострочные строки - не работают). Нелокализуемое!
#define __AUTHOR           "Eugene f2065" // описание плагина в миранде
#define __AUTHOREMAIL      "f2065@mail.ru" // описание плагина в миранде
#define __AUTHORWEB        "http://f2065.narod.ru/" // описание плагина в dll-fileinfo "CompanyName", описание плагина в миранде
#define __COPYRIGHT        "© 2010-2015 Eugene f2065" // описание плагина в dll-fileinfo "LegalCopyright", описание плагина в миранде

#define __DbModName			"HwHotKeys" // имя раздела настроек плагина в БД миранды

