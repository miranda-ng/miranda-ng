// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2009 Joe Kucera
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
//  Headers for IconLib Plugin / module support
//
// -----------------------------------------------------------------------------
#ifndef __ICONLIB_H
#define __ICONLIB_H


struct IcqIconHandle_s
{
  char *szName;
  HANDLE hIcoLib;

  HANDLE Handle();
  HICON GetIcon(bool big = false);
  void ReleaseIcon(bool big = false);
};

typedef IcqIconHandle_s *IcqIconHandle;


IcqIconHandle IconLibDefine(const char *desc, const char *section, const char *module, const char *ident, const TCHAR *def_file, int def_idx);
void IconLibRemove(IcqIconHandle *phIcon);



#endif /* __ICONLIB_H */
