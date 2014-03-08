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

{$IFNDEF M_FILE}
{$DEFINE M_FILE}

const
  {
    wParam : TMCONTACT
    lParam : 0
    Affects: Brings up the send file dialog for a contact, see notes
    Returns: returns file selection dialog's handle or 0 on error
    Notes  : Returns immediately without waiting for the send
  }
  MS_FILE_SENDFILE:PAnsiChar = 'SRFile/SendCommand';

  {
    wParam : TMCONTACT
    lParam : pointer to an array of PAnsiChar's the first nil item
             terminates the list -- see notes
    Affects: Brings up the send file dialog with specifieed files already chosen
             the user is not prevented from editing the list
    Returns: returns file selection dialog's handle or 0 on error
             returns immediately without waiting for the send to finish
    Notes  : both directories and files can be given
    Version: v0.1.2.1+
  }
  MS_FILE_SENDSPECIFICFILES:PAnsiChar = 'SRFile/SendSpecificFiles';

{
  v0.9.0+
  wParam=(WPARAM)(HANDLE)hContact
  lParam=(LPARAM)(const TCHAR**)ppFiles
  returns file selection dialog's handle or 0 on error
  returns immediately, without waiting for the send
}
  MS_FILE_SENDSPECIFICFILEST:PAnsiChar = 'SRFile/SendSpecificFilesT';

  {
    wParam : TMCONTACT
    lParam : Pointer to a buffer
    Affects: returns the received files folder for a contact, the buffer
             should be at least MAX_PATH long (defined with WinAPI),
             the returned path may not exist -- see notes
    Returns: Returns 0 on success [non zero] on failure
    notes  : If TMCONTACT is NULL(0) the path returned is the path
             without the postfix contact name.
    Version: v0.1.2.2+
  }
  MS_FILE_GETRECEIVEDFILESFOLDER:PAnsiChar = 'SRFile/GetReceivedFilesFolder';

  {
    wParam : TMCONTACT
    lParam : file selection dialog's window
    Notes  : notifies a caller about file send start
  }
  ME_FILEDLG_SUCCEEDED:PAnsiChar = 'SRFile/Dlg/Succeeded';

  {
    wParam : TMCONTACT
    lParam : file selection dialog's window
    Notes  : notifies a caller about file send cancellation
  }
  ME_FILEDLG_CANCELED:PAnsiChar = 'SRFile/Dlg/Canceled';

{$ENDIF}
