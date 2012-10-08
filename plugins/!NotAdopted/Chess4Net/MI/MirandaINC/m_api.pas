(*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2003 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

*)

{$IFDEF FPC}
    {$PACKRECORDS C}
    {$MODE Delphi}
{$ENDIF}

unit m_api;

interface

uses

    m_globaldefs, windows;

    {$include m_plugins.inc}
    {$include m_system.inc}
    {$include m_database.inc}
    {$include m_findadd.inc}
    {$include m_awaymsg.inc}
    {$include m_email.inc}
    {$include m_history.inc}
    {$include m_message.inc}
    {$include m_url.inc}
    {$include newpluginapi.inc}
    {$include m_clui.inc}
    {$include m_ignore.inc}
    {$include m_skin.inc}
    {$include m_file.inc}
    {$include m_netlib.inc}
    {$include m_langpack.inc}
    {$include m_clist.inc}
    {$include m_clc.inc}
    {$include m_userinfo.inc}
    {$include m_protosvc.inc}
    {$include m_options.inc}
    {$include m_icq.inc}
    {$include m_protocols.inc}
    {$include m_protomod.inc}
    {$include m_utils.inc}
    {$include m_addcontact.inc}
    {$include statusmodes.inc}
	{$include m_contacts.inc}
    {$define M_API_UNIT}
    {$include m_helpers.inc}

implementation

    {$undef M_API_UNIT}
    {$include m_helpers.inc}

end.

