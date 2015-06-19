{
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
}

{$IFNDEF M_PROTOMOD}
{$DEFINE M_PROTOMOD}

 {$IFNDEF M_PROTOCOLS}
     {$INCLUDE m_protocols.inc}
 {$ENDIF}

{
  Affect : Register a protocol module, see notes
  Returns: 0 on success, [non zero] on failure
  Notes  : This service MUST be called from your module's Load() function
           TPROTOCOLDESCRIPTOR.type can be a value other than PROTOTYPE_*
           which are used to provide a more precise positioning information
           for the contact protocol lists.
           -
           Relative values to the constants can be given, but this MUST NOT
           be done for PROTOTYPE_PROTOCOL.
}

function Proto_RegisterModule(descr:pPROTOCOLDESCRIPTOR) : int; stdcall;
                 external AppDLL name 'Proto_RegisterModule';

{
  Affect : Add the given protocol module to the chain for a contact, see notes
  Returns: 0 success, [non zero] on failure
  Notes  : The module is added to the correct positioning according to it's
           registered type.
}

function Proto_AddToContact(hContact:TMCONTACT; proto:PAnsiChar) : int; stdcall;
                 external AppDLL name 'Proto_AddToContact';

{
  Affect : Remove the given protocol name from the chain for the given contact
  Returns: 0 on success, [non zero] on failure
}

function Proto_RemoveFromContact(hContact:TMCONTACT; proto:PAnsiChar) : int; stdcall;
                 external AppDLL name 'Proto_RemoveFromContact';

{
  Affect : Call the next service in the chain for the send operation, see notes
  Return : Return value should be returned by CallService(MS_PROTO_CHAINSEND,wParam,lParam)
  Notes  : wParam MUST remain untouched, lParam is a pointer to a CSSDATA structure
           and can be modified or copid if needed.
           wParam and lParam should be the values passed to your service,
           typically your service should return ASAP.
}

function Proto_ChainSend(order:int; ccs:pCCSDATA) : INT_PTR; stdcall;
                 external AppDLL name 'Proto_ChainSend';

{
  Affect : Call the next service in the chain in this receive operation, see notes
  Return : Return value should be returned by CallService(MS_PROTO_CHAINRECV,wParam,lParam)
  Notes  : wParam MUST remain untouched, lParam is a pointer to a CSSDATA structure
           and can be modified or copied if needed.
           wParam and lParam should be the values passed to your service,
           typically your service should return ASAP.
           -
           MS_PROTO_CHAINRECV is thread safe since 0.1.2.0 -- calls
           are translated to the main thread and passed from there.
}

function Proto_ChainRecv(order:int; ccs:pCCSDATA) : INT_PTR; stdcall;
                 external AppDLL name 'Proto_ChainRecv';

{$ENDIF}
