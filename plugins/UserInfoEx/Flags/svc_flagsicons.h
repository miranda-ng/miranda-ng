/*
UserinfoEx plugin for Miranda IM

Copyright:
� 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

part of this code based on:
Miranda IM Country Flags Plugin Copyright �2006-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

===============================================================================

File name      : $HeadURL: https://userinfoex.googlecode.com/svn/trunk/Flags/svc_flagsicons.h $
Revision       : $Revision: 204 $
Last change on : $Date: 2010-09-28 16:48:38 +0400 (Вт, 28 сен 2010) $
Last change by : $Author: ing.u.horn $

===============================================================================
*/
#ifndef  _UINFOEX_FLAGSICONS_H_INCLUDED_
#define  _UINFOEX_FLAGSICONS_H_INCLUDED_

int		CountryNumberToIndex(int countryNumber);
HICON	LoadFlag(int countryNumber);

VOID InitIcons();
//VOID SvcFlagsOnModulesLoaded();
VOID UninitIcons();

#endif /* _UINFOEX_FLAGSICONS_H_INCLUDED_ */
